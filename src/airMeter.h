#pragma once
#include "mods/svc/hook.h"
#include "mods/svc/hook.hpp"
#include "mods/api.h"
#include "d/actor/d_a_alink.h"
#include "types.h"
#include "d/d_meter2_draw.h"
#include "d/d_meter2.h"
#include "Z2AudioLib/Z2SeMgr.h"

namespace AirMeter
{
DEFINE_HOOK(&dMeter2Draw_c::drawKanteraScreen, HookDrawKanteraScreen);

static int prevMaxOxygen;

ModResult init();
ModResult update();

HookAction preDrawKanteraScreen(ModContext*, void* args, void*, void*);
void postDrawKanteraScreen(ModContext*, void*, void*, void*);

}
