#pragma once
#include "mods/svc/hook.h"
#include "mods/svc/hook.hpp"
#include "mods/api.h"
#include "d/actor/d_a_alink.h"
#include "d/d_menu_save.h"
#include "types.h"

namespace Graphics
{
DEFINE_HOOK(&dScnKy_env_light_c::setLight_palno_get, HookSetLightPalnoGet);
DEFINE_HOOK(dKy_calc_color_set, HookCalcColorSet);

ModResult init();
ModResult update();

void postSetLightPalnoGet(ModContext*, void* args, void*, void*);
void postCalcColorSet(ModContext*, void* args, void*, void*);

}
