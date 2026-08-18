#include "wolfLinkSwimming.h"
#include "util.h"
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
    POST_HOOK(HookProcCoDead, postProcCoDead);
    PRE_HOOK(HookWolfSwimUpInit, preWolfSwimUpInit);
    REPLACE_HOOK(HookWolfSwimUp, replaceWolfSwimUp);
    PRE_HOOK(HookJointControl, preJointControl);
    POST_HOOK(HookSetWolfTailAngle, postSetWolfTailAngle);

    return MOD_OK;
}

ModResult WolfLinkSwimming::update() {
    daAlink_c* player = daAlink_getAlinkActorClass();

    if (!player) {
        return MOD_OK;
    }

    // if player isn't underwater, disable stationary movement
    // so you don't have to manually swim up after jumping into water
    if (!player->checkModeFlg(daAlink_c::MODE_SWIMMING)
        || player->checkNoResetFlg0(daAlink_c::FLG0_SWIM_UP))
    {
        shouldStayStill = false;
    }

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

    if (shouldStayStill && holdingSinkButton == holdingRiseButton) {
        cLib_chaseF(&player->speed.y, 0.0f, SWIM_ACCEL);
    }
    else if (holdingSinkButton) {
        shouldStayStill = true;
        swimSinking = true;
        player->offNoResetFlg0(daAlink_c::FLG0_SWIM_UP);  // set player to be underwater
        cLib_chaseF(&player->speed.y, MAX_SINK_SPEED, SWIM_ACCEL);
    }
    else if (holdingRiseButton 
      && !player->checkNoResetFlg0(daAlink_c::FLG0_SWIM_UP)) {
        shouldStayStill = true;
        swimRising = true;
        cLib_chaseF(&player->speed.y, MAX_RISE_SPEED, SWIM_ACCEL);
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
    target = cLib_minLimit(target, MAX_LOOK_UP_ANGLE);
    
    cLib_addCalcAngleS(&player->shape_angle.x, target, 3, ANGLE_MAX_STEP, ANGLE_MIN_STEP);
}


void WolfLinkSwimming::replaceWolfSwimWait(ModContext*, void* args, void* retval, void*) {
    daAlink_c* player = mods::arg<daAlink_c*>(args, 0);

    if (false) {  // todo: replace with config check later
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

    if (false) {  // todo: replace with config check later
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

void WolfLinkSwimming::postProcCoDead(ModContext*, void* args, void*, void*) {
    daAlink_c* player = mods::arg<daAlink_c*>(args, 0);
    // if player is wolf and underwater
    if (player->checkWolf()
      && player->checkModeFlg(daAlink_c::MODE_SWIMMING)
      && !player->checkNoResetFlg0(daAlink_c::FLG0_SWIM_UP)) {
        // change player to float down very slowly instead of floating up
        player->speed.y = cLib_minLimit(player->speed.y + DEAD_ACCEL, DEAD_SINK_SPEED);

        // offset buoyancy
        player->speed.y -= player->mpHIO->mWolf.mWlSwim.m.mBuoyancy;
    }
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

    if (false) {  // todo: replace with config check later
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


HookAction WolfLinkSwimming::preJointControl(ModContext*, void* args, void* retval, void*) {
    if (false) {  // todo: replace with config check later 
        return HOOK_CONTINUE;
    }

    daAlink_c* player = mods::arg<daAlink_c*>(args, 0);
    int i_jointNo = mods::arg<int>(args, 1);
    if (!(player->checkWolf() && i_jointNo >= 37 && i_jointNo <= 39)) {
        return HOOK_CONTINUE;
    }
    
    // the following ig a copypasted version of the original code, slightly simplified
    // but without descriptive var names cus i barely understand most of this anyway :)
    {
        csXyz sp18(0, 0, 0);
        int var_r27 = 0;

        //J3DTransformInfo jointTrans;
        J3DTransformInfo rootTrans = *player->field_0x2060->getOldFrameTransInfo(i_jointNo);

        Quaternion sp50;
        Quaternion sp40;
        Quaternion sp30;
        Quaternion sp20 = *player->field_0x2060->getOldFrameQuaternion(i_jointNo);

        csXyz sp10(0, 1, 2);

        //J3DTransformInfo* oldTransInfo;

        // no need for checking logic, we've already done that
        // modification: also set the x angle
        sp18.set(wolfTailAngleX[i_jointNo - 37], player->field_0x3094[i_jointNo - 37], 0);
        sp10.set(2, 1, 0);

        if (sp18.x != 0 || sp18.y != 0 || sp18.z != 0) {
            var_r27 |= 1;
            if (sp18.y != 0) {
                sp50 = *player->field_0x2060->getOldFrameQuaternion(i_jointNo);

                if (sp10.y == 0) {
                    JMAEulerToQuat(sp18.y, 0, 0, &sp40);
                }
                else if (sp10.y == 1) {
                    JMAEulerToQuat(0, sp18.y, 0, &sp40);
                }
                else {
                    JMAEulerToQuat(0, 0, sp18.y, &sp40);
                }

                mDoMtx_QuatConcat(&sp50, &sp40, &sp30);
            }
            else {
                sp30 = *player->field_0x2060->getOldFrameQuaternion(i_jointNo);
            }

            if (sp18.x != 0) {
                sp50 = sp30;

                if (sp10.x == 0) {
                    JMAEulerToQuat(sp18.x, 0, 0, &sp40);
                }
                else if (sp10.x == 1) {
                    JMAEulerToQuat(0, sp18.x, 0, &sp40);
                }
                else {
                    JMAEulerToQuat(0, 0, sp18.x, &sp40);
                }

                mDoMtx_QuatConcat(&sp50, &sp40, &sp30);
            }

            if (sp18.z != 0) {
                sp50 = sp30;

                if (sp10.z == 0) {
                    JMAEulerToQuat(sp18.z, 0, 0, &sp40);
                }
                else if (sp10.z == 1) {
                    JMAEulerToQuat(0, sp18.z, 0, &sp40);
                }
                else {
                    JMAEulerToQuat(0, 0, sp18.z, &sp40);
                }

                mDoMtx_QuatConcat(&sp50, &sp40, &sp30);
            }
        }

        if (var_r27 != 0) {
            MtxP temp_r26 = player->mpLinkModel->getAnmMtx(i_jointNo);

            J3DTransformInfo* var_r25 = player->field_0x2060->getOldFrameTransInfo(i_jointNo);

            Quaternion* spC;
            if ((var_r27 & 1)) {
                spC = &sp30;
            }
            else {
                spC = player->field_0x2060->getOldFrameQuaternion(i_jointNo);
            }

            mDoMtx_stack_c::transS(rootTrans.mTranslate.x, rootTrans.mTranslate.y, rootTrans.mTranslate.z);
            mDoMtx_stack_c::quatM(&sp20);
            mDoMtx_stack_c::inverse();
            cMtx_concat(temp_r26, mDoMtx_stack_c::get(), J3DSys::mCurrentMtx);

            MTXQuat(temp_r26, spC);

            temp_r26[0][3] = var_r25->mTranslate.x;
            temp_r26[1][3] = var_r25->mTranslate.y;
            temp_r26[2][3] = var_r25->mTranslate.z;

            cMtx_concat(J3DSys::mCurrentMtx, temp_r26, J3DSys::mCurrentMtx);
            cMtx_copy(J3DSys::mCurrentMtx, temp_r26);
        }

        *static_cast<int*>(retval) = 1;
    }
    return HOOK_SKIP_ORIGINAL;
}

void WolfLinkSwimming::postSetWolfTailAngle(ModContext*, void* args, void*, void*) {
    daAlink_c* player = mods::arg<daAlink_c*>(args, 0);
    // same logic but with x rotation instead
    s16* tailAngleX = wolfTailAngleX;
    s16* tailSwayDampX = wolfTailSwayDampX;

    s16 angleDiff = (s16)(player->shape_angle.x - playerPrevAngleX);

    for (int i = 0; i < 3; i++, tailAngleX++, tailSwayDampX++) {
        if (player->checkEndResetFlg0(daAlink_c::ERFLG0_UNK_800000)
            && !(player->checkWolf() && player->checkModeFlg(daAlink_c::MODE_SWIMMING))) {
            *tailAngleX = 0;
            *tailSwayDampX = 0;
        }
        else {
            *tailAngleX -= angleDiff;
            s16 sp38 = *tailAngleX;

            cLib_addCalcAngleS(tailAngleX, 0, 5, 500, 50);
            *tailAngleX = cLib_minMaxLimit<s16>((s16)(*tailAngleX + *tailSwayDampX), -0x2000, 0x2000);

            angleDiff = (s16)(*tailAngleX - sp38);
            *tailSwayDampX = angleDiff * 0.5f;
        }
    }

    // close enough right
    playerPrevAngleX = player->shape_angle.x;
}
