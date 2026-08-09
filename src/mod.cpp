#include "mods/hook.hpp"
#include "mods/service.hpp"
#include "mods/svc/hook.h"
#include "mods/svc/log.h"
#include "mods/svc/log.hpp"

// Game includes
#include "f_op/f_op_actor_mng.h"
#include "d/actor/d_a_alink.h"
#include "m_Do/m_Do_controller_pad.h"


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

static const float swimAccel = 1.0f;
static const float maxSinkSpeed = -10.0f;
static const float pitchAccel = 1500.0f;
static const float pitchMax = 9000.0f;
static const float pitchMin = -9000.0f;

// TODO: consider replace hooking daAlink_c::procWolfSwimWait and daAlink_c::procWolfSwimMove instead
// probably some more you can do there
static void UpdateWolfLinkSwimming() {

    daAlink_c* player = daAlink_getAlinkActorClass();
    if (!(player && player->checkWolf() && player->checkModeFlg(daAlink_c::MODE_SWIMMING)))
        return;

    bool sinking = mDoCPd_c::getHoldB(PAD_1);
    bool rising = mDoCPd_c::getHoldY(PAD_1);

    // stationary
    if (sinking == rising) {
        if (std::fabs(player->speed.y) < 1.0f) {  // prevent jittering
            player->speed.y = 0.0f;
        }
        else if (player->speed.y > 0.0f) {
            player->speed.y -= swimAccel;
        }
        else if (player->speed.y < 0.0f) {
            player->speed.y += swimAccel;
        }
    }
    else if (sinking) {
        player->offNoResetFlg0(daAlink_c::FLG0_SWIM_UP);  // surface flag
        player->speed.y = std::max(player->speed.y - swimAccel, maxSinkSpeed);
        player->shape_angle.x = std::min(player->shape_angle.x + pitchAccel, pitchMax);
        if (player->mProcID == daAlink_c::PROC_WOLF_SWIM_WAIT) {
            player->field_0x3124.x = std::min(player->field_0x3124.x + pitchAccel, pitchMax);  // field_0x3124 is head rotation
        }
    }
    else if (rising) {
        player->speed.y += swimAccel;
        // no need to cap speed, already done by the game

        if (!player->checkNoResetFlg0(daAlink_c::FLG0_SWIM_UP)) { // if underwater
            player->shape_angle.x = std::max(player->shape_angle.x - pitchAccel, pitchMin);
            if (player->mProcID == daAlink_c::PROC_WOLF_SWIM_WAIT) {
                player->field_0x3124.x = std::max(player->field_0x3124.x - pitchAccel, pitchMin);
            }
        }
    }

    // offset regular acceleration (i wish i could just change mBuoyancy but it's const)
    player->speed.y -= player->mpHIO->mWolf.mWlSwim.m.mBuoyancy;
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
