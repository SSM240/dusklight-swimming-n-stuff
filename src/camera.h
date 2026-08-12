#pragma once

#include "mods/svc/hook.h"
#include "mods/svc/hook.hpp"
#include "mods/api.h"
#include "d/actor/d_a_alink.h"
#include "types.h"

namespace Camera
{
DEFINE_HOOK(&daAlink_c::procCoDeadInit, HookProcCoDeadInit);

ModResult init();

void postLinkProcCoDeadInit(ModContext*, void* args, void*, void*);

}
