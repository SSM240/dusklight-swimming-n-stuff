#include "wolfLinkSwimming.h"
// dusklight includes
#include "mods/hook.hpp"
#include "mods/service.hpp"
#include "mods/svc/hook.h"
#include "mods/svc/log.h"
#include "mods/svc/log.hpp"
#include "mods/svc/hook.hpp"
#include "mods/api.h"
// game includes
#include "f_op/f_op_actor_mng.h"
#include "d/actor/d_a_alink.h"
#include "m_Do/m_Do_controller_pad.h"
#include "types.h"

ModResult WolfLinkSwimming::init() {
	ModResult result;

    mods::log::trace("replace-hooking daAlink_c::procWolfSwimWait...");
    result = mods::hook_replace<LinkWolfSwimWait>(replaceWolfSwimWait);
    if (result != MOD_OK) {
        mods::log::error("failed to install replaceWolfSwimWait");
        return result;
    }

    mods::log::trace("replace-hooking daAlink_c::procWolfSwimMove...");
    result = mods::hook_replace<LinkWolfSwimMove>(replaceWolfSwimMove);
    if (result != MOD_OK) {
        mods::log::error("failed to install replaceWolfSwimWait");
        return result;
    }

	return MOD_OK;
}

void WolfLinkSwimming::doWolfLinkSwimMovement(daAlink_c* player) {

    bool holdingSinkButton = mDoCPd_c::getHoldB(PAD_1);
    bool holdingRiseButton = mDoCPd_c::getHoldY(PAD_1);

    /*if (player->shape_angle.x != 0) {
        mods::log::debug("angle: {}", player->shape_angle.x);
    }*/

    swimSinking = swimRising = false;

    // stationary
    if (holdingSinkButton == holdingRiseButton) {
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
        swimSinking = true;
        player->offNoResetFlg0(daAlink_c::FLG0_SWIM_UP);  // set player to be underwater
        player->speed.y = std::max(player->speed.y - SWIM_ACCEL, MAX_SINK_SPEED);
        //player->shape_angle.x += PITCH_ACCEL;
        //player->shape_angle.x = std::min(player->shape_angle.x, PITCH_MAX);
    }
    else if (holdingRiseButton 
      && !player->checkNoResetFlg0(daAlink_c::FLG0_SWIM_UP)) {  // not on surface
        swimRising = true;
        player->speed.y += SWIM_ACCEL;
        // no need to cap speed, already done by the game
        //player->shape_angle.x -= PITCH_ACCEL;
        //player->shape_angle.x = std::max(player->shape_angle.x, PITCH_MIN);
    }

    // offset regular acceleration (mBuoyancy is const so i can't just change it directly)
    player->speed.y -= player->mpHIO->mWolf.mWlSwim.m.mBuoyancy;
}

void WolfLinkSwimming::doWolfLinkSwimAngle(daAlink_c* player) {
    // TODO: figure out better logic for this (or a better place to set it)
    // something is trying to reset angle.x to 0 but i can't find what
    player->shape_angle.x = player->speed.atan2sY_XZ();
}

void WolfLinkSwimming::replaceWolfSwimWait(ModContext*, void* args, void* retval, void*) {
    daAlink_c* player = mods::arg<daAlink_c*>(args, 0);

    if (false) {  // replace with config check later
        LinkWolfSwimWait::g_orig(player);
        return;
    }

    doWolfLinkSwimMovement(player);

    // always returns 1 so just set retval here for simplicity
    *static_cast<int*>(retval) = 1;

    // duplicate logic from original
    {
        if (player->mDemo.getDemoMode() == daPy_demo_c::DEMO_UNK_6_e
            || player->mDemo.getDemoMode() == daPy_demo_c::DEMO_UNK_8_e) {
            player->setShapeAngleToTalkActor();
            player->current.angle.y = player->shape_angle.y;
        }

        player->setSpeedAndAngleSwimWolf();
        player->setSwimUpDownOffset();

        if (player->checkWolfDashMode()) {
            player->mUnderFrameCtrl[0].setRate(player->mpHIO->mWolf.mWlSwim.m.mIdleAnmSpeed);
        }
        else {
            player->mUnderFrameCtrl[0].setRate(player->mpHIO->mWolf.mWlSwim.m.mIdleAnmSpeedWeak);
        }

        if (player->checkSwimUpAction()) {
            return;
        }

        if (player->decideDoStatusSwimWolf()) {
            return;
        }
    }

    // modification: transition to swim move state if swimming up or down
    if (player->checkInputOnR() || swimRising || swimSinking) {
        player->procWolfSwimMoveInit();
    }

    return;
}

void WolfLinkSwimming::replaceWolfSwimMove(ModContext*, void* args, void* retval, void*) {
    daAlink_c* player = mods::arg<daAlink_c*>(args, 0);

    if (false) {  // replace with config check later
        LinkWolfSwimMove::g_orig(player);
        return;
    }

    doWolfLinkSwimMovement(player);
    if (swimSinking || swimRising) {
        doWolfLinkSwimAngle(player);
    }

    // always returns 1 so just set retval here for simplicity
    *static_cast<int*>(retval) = 1;

    // duplicate logic from original
    {
        player->setSpeedAndAngleSwimWolf();

        daPy_frameCtrl_c* framectrl = player->mUnderFrameCtrl;

        if (!player->checkWolfSwimDashAnime()) {
            framectrl->setRate(player->getWolfSwimMoveAnmSpeed());
        }

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

    // modification: stay in this state if swimming up or down
    if (!player->checkInputOnR() && !player->checkWolfSwimDashAnime() && !swimRising && !swimSinking) {
        player->procWolfSwimWaitInit(0);
    }

    return;
}
