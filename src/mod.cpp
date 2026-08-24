#include <string>
#include "mods/service.hpp"
#include "mods/svc/hook.h"
#include "mods/svc/log.h"
#include "mods/svc/log.hpp"
#include "util.h"
#include "airMeter.h"
#include "audio.h"
#include "bubbles.h"
#include "camera.h"
#include "wolfLinkSwimming.h"

DEFINE_MOD();

IMPORT_SERVICE(LogService, svc_log);
IMPORT_SERVICE(HookService, svc_hook);

std::string Util::currName = "";

extern "C" {
MOD_EXPORT ModResult mod_initialize(ModError*) {
    ModResult result;
    
    INIT(AirMeter);
    INIT(Audio);
    INIT(Bubbles);
    INIT(Camera);
    INIT(WolfLinkSwimming);

    mods::log::info("mod initialized successfully!");
    return MOD_OK;
}

MOD_EXPORT ModResult mod_update(ModError*) {
    AirMeter::update();
    Audio::update();
    Bubbles::update();
    WolfLinkSwimming::update();

    return MOD_OK;
}

MOD_EXPORT ModResult mod_shutdown(ModError*) {
    return MOD_OK;
}
}
