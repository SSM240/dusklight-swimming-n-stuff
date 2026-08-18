#pragma once

#include "mods/svc/hook.h"
#include "mods/svc/hook.hpp"
#include "mods/api.h"
#include "d/actor/d_a_alink.h"
#include "types.h"

namespace WolfLinkSwimming
{
DEFINE_HOOK(&daAlink_c::procWolfSwimUpInit, HookWolfSwimUpInit);
DEFINE_HOOK(&daAlink_c::procWolfSwimUp, HookWolfSwimUp);
DEFINE_HOOK(&daAlink_c::procWolfSwimWait, HookWolfSwimWait);
DEFINE_HOOK(&daAlink_c::procWolfSwimMove, HookWolfSwimMove);
DEFINE_HOOK(&daAlink_c::procCoDead, HookProcCoDead);
DEFINE_HOOK(&daAlink_c::wolfFootBgCheck, HookWolfFootBgCheck);
DEFINE_HOOK(&daAlink_c::jointControll, HookJointControl);
DEFINE_HOOK(&daAlink_c::setWolfTailAngle, HookSetWolfTailAngle);

const float SWIM_ACCEL = 1.0f;
const float MAX_SINK_SPEED = -10.0f;
const float MAX_RISE_SPEED = 10.0f;
const s16 ANGLE_MAX_STEP = 2000;
const s16 ANGLE_MIN_STEP = 500;
const s16 MAX_LOOK_UP_ANGLE = -15000;

const f32 DEAD_ACCEL = -0.12f;
const f32 DEAD_SINK_SPEED = -2.0f;

static bool wasLowOnAir;

static bool shouldStayStill;
static bool swimSinking;
static bool swimRising;

static bool replacedState = false;
static u16 oldProcID;

static s16 playerPrevAngleX;
static s16 wolfTailAngleX[3];
static s16 wolfTailSwayDampX[3];

ModResult init();
ModResult update();

float getSwimAnimSpeedMult(daAlink_c* player);
void doWolfLinkSwimMovement(daAlink_c* player);
void doWolfLinkSwimAngle(daAlink_c* player);

HookAction preWolfSwimUpInit(ModContext*, void* args, void*, void*);
void replaceWolfSwimUp(ModContext*, void* args, void* retval, void*);
void replaceWolfSwimWait(ModContext*, void* args, void* retval, void*);
void replaceWolfSwimMove(ModContext*, void* args, void* retval, void*);
void postProcCoDead(ModContext*, void* args, void*, void*);
HookAction preWolfFootBgCheck(ModContext*, void* args, void*, void*);
void postWolfFootBgCheck(ModContext*, void* args, void*, void*);
HookAction preJointControl(ModContext*, void* args, void* retval, void*);
void postSetWolfTailAngle(ModContext*, void* args, void*, void*);
}
