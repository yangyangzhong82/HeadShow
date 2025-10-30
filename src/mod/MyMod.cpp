#include "mod/MyMod.h"

#include "ll/api/mod/RegisterHelper.h"
#include "HeadShow.h"

#include "Config/ConfigManager.h" // 引入配置管理器
#include "command.h"

namespace hs {

Entry& Entry::getInstance() {
    static Entry instance;
    return instance;
}

ll::io::Logger& logger = Entry::getInstance().getSelf().getLogger();

bool Entry::load() {
    getSelf().getLogger().debug("Loading...");
    // Code for loading the mod goes here.
    auto configPath = getSelf().getConfigDir();
    if (!std::filesystem::exists(configPath)) {
        std::filesystem::create_directories(configPath);
    }
    configPath /= "config.json";
    configPath.make_preferred();

    if (!ConfigManager::getInstance().load(configPath.string())) {
        getSelf().getLogger().error("Failed to load config file!");
        return false;
    }
    return true;
}

bool Entry::enable() {
    getSelf().getLogger().debug("Enabling...");
    // Code for enabling the mod goes here.
    RegisterCommand();
     startHeadShow();
    return true;
}

bool Entry::disable() {
    getSelf().getLogger().debug("Disabling...");
    // Code for disabling the mod goes here.
    return true;
}

} // namespace hs

LL_REGISTER_MOD(hs::Entry, hs::Entry::getInstance());
