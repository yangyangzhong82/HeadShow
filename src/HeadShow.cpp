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

#include "HeadShow.h"
namespace hs {

std::atomic<bool> gHeadShowRunning = false;

ll::coro::CoroTask<> sendHeadShowPacketTask() {
    while (gHeadShowRunning) {
        auto level = ll::service::getLevel();
        if (level) {
            std::vector<std::string> actorDataBuffers;
            for (auto entity : level->getRuntimeActorList()) {
                BinaryStream stream;
                stream.writeUnsignedVarInt64(entity->getRuntimeID().rawID, nullptr, nullptr);
                // DataItem
                stream.writeUnsignedVarInt(2, nullptr, nullptr);
                stream.writeUnsignedVarInt((uint)ActorDataIDs::Name, nullptr, nullptr);
                stream.writeUnsignedVarInt((uint)DataItemType::String, nullptr, nullptr);
                stream.writeString(entity->getTypeName(), nullptr, nullptr);
                stream.writeUnsignedVarInt((uint)ActorDataIDs::NametagAlwaysShow, nullptr, nullptr);
                stream.writeUnsignedVarInt((uint)DataItemType::Byte, nullptr, nullptr);
                stream.writeBool(true, nullptr, nullptr);
                // Other
                stream.writeUnsignedVarInt(0, nullptr, nullptr);
                stream.writeUnsignedVarInt(0, nullptr, nullptr);
                stream.writeUnsignedVarInt64(0, nullptr, nullptr);
                actorDataBuffers.push_back(std::move(stream.mBuffer));
            }

            level->forEachPlayer([&](Player& player) {
                if (player.isSimulated()) {
                    return true;
                }
                for (auto entity : level->getRuntimeActorList()) {
                    if (entity->getDimensionId() != player.getDimensionId()) {
                        continue;
                    }
                    BinaryStream stream;
                    stream.writeUnsignedVarInt64(entity->getRuntimeID().rawID, nullptr, nullptr);
                    // DataItem
                    stream.writeUnsignedVarInt(2, nullptr, nullptr);
                    stream.writeUnsignedVarInt((uint)ActorDataIDs::Name, nullptr, nullptr);
                    stream.writeUnsignedVarInt((uint)DataItemType::String, nullptr, nullptr);
                    stream.writeString(entity->getTypeName(), nullptr, nullptr);
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
        co_await std::chrono::milliseconds(50); // Default update interval
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
