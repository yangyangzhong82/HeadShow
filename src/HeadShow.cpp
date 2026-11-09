#include <algorithm>
#include <atomic>
#include <chrono>
#include <memory>
#include <utility>

#include "ll/api/service/Bedrock.h"
#include "ll/api/thread/ServerThreadExecutor.h"
#include "logger.h"
#include "mc/network/packet/SetActorDataPacket.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/actor/ActorDataIDs.h"
#include "mc/world/actor/DataItem.h"
#include "mc/world/level/Level.h"

#include "Config/ConfigManager.h"
#include "HeadShow.h"
#include "PA/PlaceholderAPI.h"
namespace hs {

namespace {

std::atomic<bool>                              gHeadShowRunning = false;
std::shared_ptr<ll::data::CancellableCallback> gHeadShowTask;

void sendHeadShowPacketTask() {
    if (!gHeadShowRunning) {
        return;
    }
    auto& config = ConfigManager::getInstance().get(); // 每次迭代都重新获取配置
    auto  level  = ll::service::getLevel();
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
                std::string entityTypeName = entity->getTypeName();
                auto        it             = config.entityNameOverrides.find(entityTypeName);
                std::string name;
                if (it != config.entityNameOverrides.end()) {
                    if (entityTypeName == "minecraft:player") {
                        std::unique_ptr<PA::IContext> ctx_ptr =
                            PA::PlayerContext::factory(static_cast<void*>(static_cast<Player*>(entity)));
                        name = PA::PA_GetPlaceholderService()->replace(it->second, ctx_ptr.get());
                    } else {
                        std::unique_ptr<PA::IContext> ctx_ptr = PA::ActorContext::factory(static_cast<void*>(entity));
                        name = PA::PA_GetPlaceholderService()->replace(it->second, ctx_ptr.get());
                    }
                } else {
                    continue; // 如果实体不在配置里，就不对其设置显示
                }
                // Pack a snapshot without consuming the actor's dirty metadata.
                SetActorDataPacket packet(entity->getRuntimeID(), *entity->mEntityData, nullptr, uint64{0}, true);
                auto&              items = *packet.mPackedItems;
                items.clear();
                items.push_back(DataItem::create(ActorDataIDs::Name, std::move(name)));
                items.push_back(DataItem::create(ActorDataIDs::NametagAlwaysShow, true));
                packet.sendTo(player);
            }
            return true;
        });
    }
    if (gHeadShowRunning) {
        gHeadShowTask = ll::thread::ServerThreadExecutor::getDefault().executeAfter(
            sendHeadShowPacketTask,
            std::chrono::milliseconds(std::max(1, config.refreshIntervalMs))
        );
    }
}

} // namespace

void startHeadShow() {
    if (gHeadShowRunning.exchange(true)) {
        return;
    }
    logger.debug("Starting HeadShow packet task...");
    gHeadShowTask = ll::thread::ServerThreadExecutor::getDefault().executeAfter(
        sendHeadShowPacketTask,
        std::chrono::milliseconds::zero()
    );
}

void stopHeadShow() {
    if (gHeadShowRunning.exchange(false)) {
        if (gHeadShowTask) {
            gHeadShowTask->cancel();
            gHeadShowTask.reset();
        }
        logger.debug("Stopping HeadShow packet task...");
    }
}

} // namespace hs
