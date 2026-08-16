#pragma once

#include "mods/svc/hook.h"
#include "mods/svc/hook.hpp"
#include "mods/api.h"
#include "d/actor/d_a_alink.h"
#include "types.h"
#include "d/d_camera.h"

namespace Camera
{
DEFINE_HOOK(&daAlink_c::procCoDead, HookProcCoDead);
DEFINE_HOOK(&dCamera_c::checkForceLockTarget, HookCheckForceLockTarget);

ModResult init();

static bool shouldAllowCameraControl;

static f32 mLastPosX;
static f32 mLastPosY;
static f32 mLastValue;

void postLinkProcCoDead(ModContext*, void* args, void*, void*);
void postCheckForceLockTarget(ModContext*, void* args, void*, void*);

}
