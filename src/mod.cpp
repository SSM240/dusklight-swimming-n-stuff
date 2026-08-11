#include "mods/hook.hpp"
#include "mods/service.hpp"
#include "mods/svc/hook.h"
#include "mods/svc/log.h"
#include "mods/svc/log.hpp"
#include "wolfLinkSwimming.h"
#include "bubbles.h"
#include "airMeter.h"

DEFINE_MOD();

IMPORT_SERVICE(LogService, svc_log);
IMPORT_SERVICE(HookService, svc_hook);

extern "C" {
MOD_EXPORT ModResult mod_initialize(ModError*) {
    ModResult result = MOD_OK;
    
    // todo: figure out better error reporting here?
    WolfLinkSwimming::init();
    AirMeter::init();
    Bubbles::init();

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
