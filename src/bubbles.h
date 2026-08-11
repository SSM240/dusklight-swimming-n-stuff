#pragma once
#include "mods/svc/hook.h"
#include "mods/svc/hook.hpp"
#include "mods/api.h"
#include "d/actor/d_a_alink.h"
#include "types.h"

namespace Bubbles
{
DEFINE_HOOK(&daAlink_c::setEmitterPolyColor, HookSetEmitterPolyColor);
DEFINE_HOOK(&daAlink_c::setEffect, HookSetEffect);

static int deathTimer = 0;
static float bubbleFrequency;

ModResult init();
ModResult update();

HookAction preSetEmitter(ModContext*, void* args, void* retval, void*);
void postSetEffect(ModContext*, void* args, void* retval, void*);
}
