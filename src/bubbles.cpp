#include "bubbles.h"
#include "airMeter.h"
#include "util.h"
#include <cmath>
#include "mods/svc/hook.h"
#include "mods/svc/hook.hpp"
#include "mods/svc/log.h"
#include "mods/svc/log.hpp"
#include "mods/api.h"
#include "d/actor/d_a_alink.h"
#include "d/actor/d_a_player.h"
#include "d/d_particle_name.h"
#include "types.h"
#include "mtx/GeoTypes.h"
#include "d/d_particle.h"
#include "m_Do/m_Do_mtx.h"
#include "SSystem/SComponent/c_math.h"
#include "SSystem/SComponent/c_xyz.h"
#include "JSystem/JParticle/JPAEmitter.h"
#include "d/d_com_inf_game.h"

ModResult Bubbles::init() {
    ModResult result;

    PRE_HOOK(HookSetEmitterPolyColor, preSetEmitter);
    POST_HOOK(HookSetEffect, postSetEffect);

    return MOD_OK;
}

// dynamically set frequency of bubbles based on remaining oxygen
ModResult Bubbles::update() {
    float oxygenPercent = (f32)dComIfGp_getOxygen() / (f32)dComIfGp_getMaxOxygen();

    daAlink_c* player = daAlink_getAlinkActorClass();

    // gradually taper off bubble frequency when player dies
    if (player && player->mProcID == daAlink_c::PROC_DEAD) {
        deathTimer++;
        if (deathTimer > 45) {
            bubbleFrequency *= 0.95f;
        }
    }
    else {
        deathTimer = 0;
        if (oxygenPercent > 0.5f) {
            bubbleFrequency = 0.05f;
        }
        else {
            bubbleFrequency = powf(1.0f - 2.0f * oxygenPercent, 3.0f) / 2.0f + 0.05f;
        }
    }

    return MOD_OK;
}


HookAction Bubbles::preSetEmitter(ModContext*, void* args, void* retval, void*) {
    daAlink_c* player = mods::arg<daAlink_c*>(args, 0);
    u16 i_effName = mods::arg<u16>(args, 2);
    if (i_effName != ID_ZI_J_LK_ABUKU_A) {
        return HOOK_CONTINUE;
    }
    if (cM_rnd() < bubbleFrequency) {
        return HOOK_CONTINUE;
    }
    retval = NULL;
    return HOOK_SKIP_ORIGINAL;
}

void Bubbles::postSetEffect(ModContext*, void* args, void* retval, void*) {
    daAlink_c* player = mods::arg<daAlink_c*>(args, 0);
    // spawn bubble particles as normal if you're wolf link
    if (player->checkWolf()) {
        Vec position;
        // 7 is closer to the mouth than the default
        mDoMtx_multVecZero(player->mpLinkModel->getAnmMtx(7), &position);
        if (position.y < (player->mWaterY - 50.0f)) {
            JPABaseEmitter* emitter = player->setEmitterPolyColor(
                &player->field_0x31bc, ID_ZI_J_LK_ABUKU_A,
                player->mLinkAcch.m_wtr, (cXyz*)&position, &player->shape_angle);
            if (emitter != NULL) {
                emitter->setParticleCallBackPtr(dPa_control_c::getWaterBubblePcallBack());
            }
        }
    }
}
