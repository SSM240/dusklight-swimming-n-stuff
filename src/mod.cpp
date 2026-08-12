#include "mods/service.hpp"
#include "mods/svc/hook.h"
#include "mods/svc/log.h"
#include "mods/svc/log.hpp"
#include "util.h"
#include "airMeter.h"
#include "bubbles.h"
#include "camera.h"
#include "misc.h"
#include "wolfLinkSwimming.h"

DEFINE_MOD();

IMPORT_SERVICE(LogService, svc_log);
IMPORT_SERVICE(HookService, svc_hook);

extern "C" {
MOD_EXPORT ModResult mod_initialize(ModError*) {
    ModResult result;
    
    INIT(AirMeter);
    INIT(Bubbles);
    INIT(Camera);
    INIT(Misc);
    INIT(WolfLinkSwimming);

    mods::log::info("swimming_n_stuff initialized");
    return MOD_OK;
}

MOD_EXPORT ModResult mod_update(ModError*) {
    AirMeter::update();
    Bubbles::update();

    return MOD_OK;
}

MOD_EXPORT ModResult mod_shutdown(ModError*) {
    return MOD_OK;
}
}
