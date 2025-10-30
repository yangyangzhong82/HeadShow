#include "command.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/server/commands/PlayerCommandOrigin.h"
#include "mc/world/actor/player/Player.h"
#include <string>
#include <utility>
#include <vector>


#include "Config/ConfigManager.h" // 引入配置管理器
#include "logger.h"               // 引入logger


namespace hs {

// 定义一个用于重新加载配置的函数
void ReloadCommand(CommandOrigin const& origin, CommandOutput& output) {
    if (ConfigManager::getInstance().reload()) {
        output.success("配置文件已成功重新加载！");
        hs::logger.info("配置文件已成功重新加载！");
    } else {
        output.error("配置文件重新加载失败！请检查日志获取更多信息。");
        hs::logger.error("配置文件重新加载失败！");
    }
}

void RegisterCommand() {
    using ll::command::CommandHandle;
    using ll::command::CommandRegistrar;
    using ll::command::EmptyParam; // 引入 EmptyParam

    auto& registrar = CommandRegistrar::getInstance();
    auto& command = registrar.getOrCreateCommand("hs", "重新加载hs配置", CommandPermissionLevel::GameDirectors);

    // 注册 'reload' 命令
    command
        .overload<EmptyParam>() 
        .text("reload")
        .execute([](CommandOrigin const& origin, CommandOutput& output) { 
            ReloadCommand(origin, output);
        });
}

} // namespace hs
