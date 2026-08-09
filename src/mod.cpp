#include "mods/hook.hpp"
#include "mods/service.hpp"
#include "mods/svc/hook.h"
#include "mods/svc/log.h"

// Game includes
#include "f_op/f_op_actor_mng.h"
#include "d/actor/d_a_alink.h"
#include "m_Do/m_Do_controller_pad.h"

#include <format>

DEFINE_MOD();

IMPORT_SERVICE(LogService, svc_log);
IMPORT_SERVICE(HookService, svc_hook);

//DEFINE_HOOK(&daAlink_c::initGravity, InitGravity);



template <typename T> void DebugLog(T val) {
    svc_log->debug(mod_ctx, std::format("{}", val).c_str());
}
template <typename T> void DebugLog(std::string name, T val) {
    svc_log->debug(mod_ctx, std::format("{}: {}", name, val).c_str());
}

// this hook did not seem to do what i wanted it to

//static HookAction on_init_gravity(ModContext*, void* args, void*, void*) {
//    daAlink_c* player = mods::arg_ref<daAlink_c*>(args, 0);
//    DebugLog("is wolf", player->checkWolf());
//    if (player->checkWolf() 
//      && player->checkModeFlg(player->MODE_SWIMMING) 
//      && !player->checkHeavyStateOn(TRUE, TRUE)) {
//        f32 gravity = 1.5f;
//        f32 max_fall_speed = -10.0f;
//        player->setSpecialGravity(gravity, max_fall_speed, TRUE);
//        return HOOK_SKIP_ORIGINAL;
//    }
//    return HOOK_CONTINUE;
//}

static void UpdateWolfLinkSwimming() {
    daAlink_c* player = daAlink_getAlinkActorClass();
    if (!(player && player->checkWolf() && player->checkModeFlg(player->MODE_SWIMMING)))
        return;

    bool sinking = mDoCPd_c::getHoldB(PAD_1);
    bool rising = mDoCPd_c::getHoldY(PAD_1);

    if (sinking) {
        player->speed.y = -11.5f;
        player->offNoResetFlg0(player->FLG0_SWIM_UP);
    }
    else if (rising) {
        player->speed.y = 10.0f;
    }
    else {
        player->speed.y = -1.5f;
    }
}


extern "C" {
MOD_EXPORT ModResult mod_initialize(ModError*) {
    //ModResult result;
    //// Installs a pre hook on daAlink_c::initGravity.
    //result = mods::hook_add_pre<InitGravity>(svc_hook, on_init_gravity);
    //if (result != MOD_OK) {
    //    svc_log->error(mod_ctx, "failed to install on_init_gravity");
    //    return result;
    //}

    svc_log->info(mod_ctx, "swimming_n_stuff initialized");
    return MOD_OK;
}

MOD_EXPORT ModResult mod_update(ModError*) {
    UpdateWolfLinkSwimming();

    return MOD_OK;
}

MOD_EXPORT ModResult mod_shutdown(ModError*) {
    return MOD_OK;
}
}
