#include "mods/hook.hpp"
#include "mods/service.hpp"
#include "mods/svc/hook.h"
#include "mods/svc/log.h"
#include "mods/svc/log.hpp"
#include "wolfLinkSwimming.h"
#include "bubbles.h"


DEFINE_MOD();

IMPORT_SERVICE(LogService, svc_log);
IMPORT_SERVICE(HookService, svc_hook);

//DEFINE_HOOK(&daAlink_c::initGravity, InitGravity);

// this hook did not seem to do what i wanted it to

//static HookAction on_init_gravity(ModContext*, void* args, void*, void*) {
//    daAlink_c* player = mods::arg_ref<daAlink_c*>(args, 0);
//    
//    if (player->checkWolf() 
//      && player->checkModeFlg(player->MODE_SWIMMING) 
//      && !player->checkHeavyStateOn(TRUE, TRUE)) {
//        f32 gravity = 0.0f;
//        f32 max_fall_speed = 0.0f;
//        player->setSpecialGravity(gravity, max_fall_speed, TRUE);
//        return HOOK_SKIP_ORIGINAL;
//    }
//    return HOOK_CONTINUE;
//}

extern "C" {
MOD_EXPORT ModResult mod_initialize(ModError*) {
    ModResult result = MOD_OK;
    
    // todo: figure out better error reporting here?
    WolfLinkSwimming::init();
    Bubbles::init();

    svc_log->info(mod_ctx, "swimming_n_stuff initialized");
    return MOD_OK;
}

MOD_EXPORT ModResult mod_update(ModError*) {
    return MOD_OK;
}

MOD_EXPORT ModResult mod_shutdown(ModError*) {
    return MOD_OK;
}
}
