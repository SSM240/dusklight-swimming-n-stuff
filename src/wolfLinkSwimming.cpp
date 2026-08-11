#include "wolfLinkSwimming.h"
#include "util.h"
#include <algorithm>
#include <cmath>
#include "mods/hook.hpp"
#include "mods/svc/hook.h"
#include "mods/svc/log.h"
#include "mods/svc/log.hpp"
#include "mods/svc/hook.hpp"
#include "mods/api.h"
#include "d/actor/d_a_alink.h"
#include "d/actor/d_a_player.h"
#include "d/d_com_inf_game.h"
#include "m_Do/m_Do_controller_pad.h"
#include "types.h"
#include "SSystem/SComponent/c_lib.h"
#include "SSystem/SComponent/c_math.h"

ModResult WolfLinkSwimming::init() {
    ModResult result;

    REPLACE_HOOK(HookWolfSwimWait, replaceWolfSwimWait);
    REPLACE_HOOK(HookWolfSwimMove, replaceWolfSwimMove);
    PRE_HOOK(HookWolfFootBgCheck, preWolfFootBgCheck);
    POST_HOOK(HookWolfFootBgCheck, postWolfFootBgCheck);
    PRE_HOOK(HookWolfSwimUpInit, preWolfSwimUpInit);
    REPLACE_HOOK(HookWolfSwimUp, replaceWolfSwimUp);

    return MOD_OK;
}

float WolfLinkSwimming::getSwimAnimSpeedMult(daAlink_c* player) {
    float mult = 1.0f;
    if (!player->checkNoResetFlg0(daAlink_c::FLG0_SWIM_UP)) {
        float oxygenPercent = (f32)dComIfGp_getOxygen() / (f32)dComIfGp_getMaxOxygen();
        if (oxygenPercent > 0.5f) {
            mult *= 0.5f;
        }
        else {
            mult *= (1.0f - oxygenPercent) * 1.2f;
        }
    }
    return mult;
}


void WolfLinkSwimming::doWolfLinkSwimMovement(daAlink_c* player) {

    bool holdingSinkButton = mDoCPd_c::getHoldB(PAD_1);
    bool holdingRiseButton = mDoCPd_c::getHoldY(PAD_1);

    swimSinking = swimRising = false;
    
    // don't try to touch player's speed if they're not trying to dive or swim up
    // so jumping into water still raises you to the surface
    if (player->checkNoResetFlg0(daAlink_c::FLG0_SWIM_UP)) {
        shouldStayStill = false;
    }

    // stationary
    if (shouldStayStill && holdingSinkButton == holdingRiseButton) {
        if (std::fabs(player->speed.y) < 1.0f) {  // prevent jittering
            player->speed.y = 0.0f;
        }
        else if (player->speed.y > 0.0f) {
            player->speed.y -= SWIM_ACCEL;
        }
        else if (player->speed.y < 0.0f) {
            player->speed.y += SWIM_ACCEL;
        }
    }
    else if (holdingSinkButton) {
        shouldStayStill = true;
        swimSinking = true;
        player->offNoResetFlg0(daAlink_c::FLG0_SWIM_UP);  // set player to be underwater
        player->speed.y = std::max(player->speed.y - SWIM_ACCEL, MAX_SINK_SPEED);
    }
    else if (holdingRiseButton 
      && !player->checkNoResetFlg0(daAlink_c::FLG0_SWIM_UP)) {
        shouldStayStill = true;
        swimRising = true;
        player->speed.y += SWIM_ACCEL;
        // no need to cap speed, already done by the game
    }

    // offset regular acceleration
    if (shouldStayStill && !player->checkNoResetFlg0(daAlink_c::FLG0_SWIM_UP)) {
        player->speed.y -= player->mpHIO->mWolf.mWlSwim.m.mBuoyancy;
    }
}


void WolfLinkSwimming::doWolfLinkSwimAngle(daAlink_c* player) {
    s16 target;
    if (player->checkNoResetFlg0(daAlink_c::FLG0_SWIM_UP)) {  // on surface
        target = 0;
    }
    else {
        float targetYSpeed = swimRising ? MAX_RISE_SPEED
            : swimSinking ? MAX_SINK_SPEED
            : 0.0f;
        target = cM_atan2s(-targetYSpeed, player->speed.absXZ());
    }
    // straight up looks slightly odd to me for some reason
    if (target < -15000) {
        target = -15000;
    }
    
    cLib_addCalcAngleS(&player->shape_angle.x, target, 3, 2000, 500);
}


void WolfLinkSwimming::replaceWolfSwimWait(ModContext*, void* args, void* retval, void*) {
    daAlink_c* player = mods::arg<daAlink_c*>(args, 0);

    if (false) {  // replace with config check later
        swimSinking = swimRising = false;
        HookWolfSwimWait::g_orig(player);
        return;
    }

    // always returns 1 so just set retval here for simplicity
    *static_cast<int*>(retval) = 1;

    // unchanged logic from original
    {
        if (player->mDemo.getDemoMode() == daPy_demo_c::DEMO_UNK_6_e
            || player->mDemo.getDemoMode() == daPy_demo_c::DEMO_UNK_8_e) {
            player->setShapeAngleToTalkActor();
            player->current.angle.y = player->shape_angle.y;
        }

        player->setSpeedAndAngleSwimWolf();
        player->setSwimUpDownOffset();
    }

    // modification: change swim animation speed
    float rate = player->checkWolfDashMode()
        ? player->mpHIO->mWolf.mWlSwim.m.mIdleAnmSpeed
        : player->mpHIO->mWolf.mWlSwim.m.mIdleAnmSpeedWeak;
    rate *= WolfLinkSwimming::getSwimAnimSpeedMult(player);
    player->mUnderFrameCtrl->setRate(rate);

    // unchanged logic from original
    {
        if (player->checkSwimUpAction()) {
            return;
        }

        if (player->decideDoStatusSwimWolf()) {
            return;
        }
    }

    // modification: handle custom up/down movement
    doWolfLinkSwimMovement(player);
    doWolfLinkSwimAngle(player);

    // modification: transition to swim move state if swimming up or down
    if (player->checkInputOnR() || swimRising || swimSinking) {
        player->procWolfSwimMoveInit();
    }

    return;
}


void WolfLinkSwimming::replaceWolfSwimMove(ModContext*, void* args, void* retval, void*) {
    daAlink_c* player = mods::arg<daAlink_c*>(args, 0);

    if (false) {  // replace with config check later
        swimSinking = swimRising = false;
        HookWolfSwimMove::g_orig(player);
        return;
    }

    // always returns 1 so just set retval here for simplicity
    *static_cast<int*>(retval) = 1;

    player->setSpeedAndAngleSwimWolf();

    if (!player->checkWolfSwimDashAnime()) {
        // modification: change swim animation speed
        float rate = player->getWolfSwimMoveAnmSpeed();
        if (swimSinking || swimRising) {
            rate = player->checkWolfDashMode()
              ? player->mpHIO->mWolf.mWlSwim.m.mMoveMaxAnmSpeed
              : player->mpHIO->mWolf.mWlSwim.m.mMoveMaxAnmSpeedWeak;
        }
        rate *= getSwimAnimSpeedMult(player);
        player->mUnderFrameCtrl->setRate(rate);
    }

    // bunch of unchanged logic from original
    {
        if (player->checkSwimUpAction()) {
            return;
        }

        if (player->decideDoStatusSwimWolf()) {
            return;
        }

        if (player->checkAnmEnd(player->mUnderFrameCtrl) && player->checkWolfSwimDashAnime()) {
            player->setSingleAnimeWolfBaseSpeed(player->WANM_SWIM, player->getWolfSwimMoveAnmSpeed(),
                player->mpHIO->mWolf.mWlSwim.m.mMoveInterp);

            if (player->field_0x2fc7 == 1 || player->field_0x2fc7 == 3) {
                player->field_0x30d2 = player->mpHIO->mWolf.mWlMove.m.mADashCooldownTimeSlow;
            }
            else if (player->field_0x2fc7 == 2) {
                player->field_0x30d2 = player->mpHIO->mWolf.mWlMove.m.mADashCooldownTimeSlow2;
            }
            else {
                player->field_0x30d2 = player->mpHIO->mWolf.mWlMove.m.mADashCooldownTime;
            }
        }

        if (player->checkFrontWallTypeAction()) {
            player->swimOutAfter(1);
            return;
        }
    }

    // modification: handle custom up/down movement
    doWolfLinkSwimMovement(player);
    doWolfLinkSwimAngle(player);

    // modification: stay in this state if swimming up or down
    if (!player->checkInputOnR() && !player->checkWolfSwimDashAnime() && !swimRising && !swimSinking) {
        player->procWolfSwimWaitInit(0);
    }

    return;
}


// hack: this function is coded not to touch player pitch in PROC_WOLF_CARGO_CARRY
// so by replacing the proc ID temporarily we can disable this in other states also
HookAction WolfLinkSwimming::preWolfFootBgCheck(ModContext*, void* args, void*, void*) {
    daAlink_c* player = mods::arg<daAlink_c*>(args, 0);
    u16 procID = player->mProcID;
    if ((procID == daAlink_c::PROC_WOLF_SWIM_MOVE || procID == daAlink_c::PROC_WOLF_SWIM_WAIT)
      && !player->checkNoResetFlg0(daAlink_c::FLG0_SWIM_UP)) {
        replacedState = true;
        oldProcID = player->mProcID;
        player->mProcID = daAlink_c::PROC_WOLF_CARGO_CARRY;
    }
    return HOOK_CONTINUE;
}
// restore the proc ID if we changed it in preWolfFootBgCheck
void WolfLinkSwimming::postWolfFootBgCheck(ModContext*, void* args, void*, void*) {
    daAlink_c* player = mods::arg<daAlink_c*>(args, 0);
    if (replacedState) {
        player->mProcID = oldProcID;
    }
    replacedState = false;
}


// keep track of whether we're low on air at the moment we surface
HookAction WolfLinkSwimming::preWolfSwimUpInit(ModContext*, void* args, void*, void*) {
    float oxygenPercent = (f32)dComIfGp_getOxygen() / (f32)dComIfGp_getMaxOxygen();
    wasLowOnAir = oxygenPercent < 0.5f;
    return HOOK_CONTINUE;
}
void WolfLinkSwimming::replaceWolfSwimUp(ModContext*, void* args, void* retval, void*) {
    daAlink_c* player = mods::arg<daAlink_c*>(args, 0);

    if (false) {  // replace with config check later
        HookWolfSwimWait::g_orig(player);
        return;
    }

    daPy_frameCtrl_c* frameCtrl = player->mUnderFrameCtrl;
    player->setNormalSpeedF(0.0f, player->mpHIO->mWolf.mWlSwim.m.mDeceleration);

    // modification: change animation to be unskippable if low on air
    float cancelFrame = wasLowOnAir
        ? 20.0f
        : player->mpHIO->mWolf.mWlSwim.m.mSurfacingAnm.mCancelFrame;
    if (player->checkAnmEnd(frameCtrl)) {
        player->procWolfSwimWaitInit(0);
    }
    else if (player->checkInputOnR() && frameCtrl->getFrame() > cancelFrame) {
        player->procWolfSwimMoveInit();
    }
    else {
        player->current.pos.y = player->mWaterY;
    }

    *static_cast<int*>(retval) = 1;
}
