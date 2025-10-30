#include "mod/MyMod.h"

#include "ll/api/mod/RegisterHelper.h"
#include "HeadShow.h"
namespace hs {

Entry& Entry::getInstance() {
    static Entry instance;
    return instance;
}

ll::io::Logger& logger = Entry::getInstance().getSelf().getLogger();

bool Entry::load() {
    getSelf().getLogger().debug("Loading...");
    // Code for loading the mod goes here.
    return true;
}

bool Entry::enable() {
    getSelf().getLogger().debug("Enabling...");
    // Code for enabling the mod goes here.
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
