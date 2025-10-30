#pragma once

#include <map>
#include <string>
#include <vector>


struct Config {
    int                                version             = 1;
    bool                               enablePlugin        = true;
    int                                displayDistance     = 30;
    std::map<std::string, std::string> entityNameOverrides = {
        {"minecraft:player", "玩家"      },
        {"minecraft:item",   "{actor_name}"}
    };
    int refreshIntervalMs = 50;
};
