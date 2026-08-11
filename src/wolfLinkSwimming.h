#pragma once

#include "mods/hook.hpp"
#include "mods/service.hpp"
#include "mods/svc/hook.h"
#include "mods/svc/log.h"
#include "mods/svc/log.hpp"
#include "mods/svc/hook.hpp"
#include "mods/api.h"
#include "d/actor/d_a_alink.h"
#include "types.h"

namespace WolfLinkSwimming
{
DEFINE_HOOK(&daAlink_c::procWolfSwimWait, HookWolfSwimWait);
DEFINE_HOOK(&daAlink_c::procWolfSwimMove, HookWolfSwimMove);
DEFINE_HOOK(&daAlink_c::wolfFootBgCheck, HookWolfFootBgCheck);
DEFINE_HOOK(&daAlink_c::getWolfSwimMoveAnmSpeed, HookWolfSwimMoveAnmSpeed);

static const float SWIM_ACCEL = 1.0f;
static const float MAX_SINK_SPEED = -10.0f;
static const float MAX_RISE_SPEED = 10.0f; // unused, for reference
static const s16 PITCH_ACCEL = 3000;
static const s16 PITCH_MAX = 9000;
static const s16 PITCH_MIN = -6000;

static bool swimSinking;
static bool swimRising;

static bool replacedState = false;
static u16 oldProcID;

ModResult init();
void doWolfLinkSwimMovement(daAlink_c* player);
void doWolfLinkSwimAngle(daAlink_c* player);

void replaceWolfSwimWait(ModContext*, void* args, void* retval, void*);
void replaceWolfSwimMove(ModContext*, void* args, void* retval, void*);
HookAction preWolfFootBgCheck(ModContext*, void* args, void*, void*);
void postWolfFootBgCheck(ModContext*, void* args, void*, void*);
void replaceWolfSwimMoveAnmSpeed(ModContext*, void* args, void* retval, void*);
}
