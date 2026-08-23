#pragma once
#include "mods/svc/hook.h"
#include "mods/svc/hook.hpp"
#include "mods/api.h"
#include "d/actor/d_a_alink.h"
#include "d/d_menu_save.h"
#include "Z2AudioLib/Z2SeqMgr.h"
#include "types.h"

namespace Audio
{
DEFINE_HOOK(&Z2SeqMgr::checkBgmIDPlaying, HookCheckBgmIDPlaying);

ModResult init();

ModResult update();

HookAction preCheckBgmIDPlaying(ModContext*, void* args, void* retval, void*);
}
