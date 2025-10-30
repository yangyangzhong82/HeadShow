#include "ll/api/chrono/GameChrono.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/thread/ServerThreadExecutor.h"
#include "logger.h"
#include "mc/network/NetworkPeer.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/actor/ActorDataIDs.h"
#include "mc/world/level/Level.h"
#include "NetworkPacket.h"
#include "mc/legacy/ActorRuntimeID.h"
#include "mc/world/actor/DataItemType.h"

#include "PA/PlaceholderAPI.h"
#include "HeadShow.h"
#include "Config/ConfigManager.h"
namespace hs {

std::atomic<bool> gHeadShowRunning = false;

ll::coro::CoroTask<> sendHeadShowPacketTask() {
    while (gHeadShowRunning) {
        auto& config = ConfigManager::getInstance().get(); // 每次迭代都重新获取配置
        auto level = ll::service::getLevel();
        if (level) {
            level->forEachPlayer([&](Player& player) {
                if (player.isSimulated()) {
                    return true;
                }

                for (auto entity : level->getRuntimeActorList()) {
                    if (entity->getDimensionId() != player.getDimensionId()) {
                        continue;
                    }
                    if (entity->getPosition().distanceTo(player.getPosition()) > config.displayDistance) {
                        continue;
                    }
                    BinaryStream stream;
                    stream.writeUnsignedVarInt64(entity->getRuntimeID().rawID, nullptr, nullptr);
                    // DataItem
                    stream.writeUnsignedVarInt(2, nullptr, nullptr);
                    stream.writeUnsignedVarInt((uint)ActorDataIDs::Name, nullptr, nullptr);
                    stream.writeUnsignedVarInt((uint)DataItemType::String, nullptr, nullptr);
                    std::string entityTypeName = entity->getTypeName();
                    auto it = config.entityNameOverrides.find(entityTypeName);
                    if (it != config.entityNameOverrides.end()) {
                        if (entityTypeName == "minecraft:player") {
                            PA::PlayerContext ctx;
                            ctx.player = &player; // Use the current player for player-specific placeholders
                            stream.writeString(PA::PA_GetPlaceholderService()->replace(it->second, &ctx), nullptr, nullptr);
                        } else {
                            PA::ActorContext ctx;
                            ctx.actor = entity;
                            stream.writeString(PA::PA_GetPlaceholderService()->replace(it->second, &ctx), nullptr, nullptr);
                        }
                    } else {
                        stream.writeString(entityTypeName, nullptr, nullptr);
                    }
                    stream.writeUnsignedVarInt((uint)ActorDataIDs::NametagAlwaysShow, nullptr, nullptr);
                    stream.writeUnsignedVarInt((uint)DataItemType::Byte, nullptr, nullptr);
                    stream.writeBool(true, nullptr, nullptr);
                    // Other
                    stream.writeUnsignedVarInt(0, nullptr, nullptr);
                    stream.writeUnsignedVarInt(0, nullptr, nullptr);
                    stream.writeUnsignedVarInt64(0, nullptr, nullptr);
                    NetworkPacket<MinecraftPacketIds::SetActorData> addPkt(stream.mBuffer);
                    addPkt.sendTo(player);
                }
                return true;
            });
        }
        // TODO: Make update interval configurable
        co_await std::chrono::milliseconds(config.refreshIntervalMs); // Default update interval
    }
    co_return;
}

void startHeadShow() {
    gHeadShowRunning = true;
    logger.debug("Starting HeadShow packet task...");
    ll::coro::keepThis(sendHeadShowPacketTask).launch(ll::thread::ServerThreadExecutor::getDefault());
}

void stopHeadShow() {
    if (gHeadShowRunning) {
        gHeadShowRunning = false;
        logger.debug("Stopping HeadShow packet task...");
    }
}

} // namespace HeadShow
