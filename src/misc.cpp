#include "misc.h"
#include "util.h"
#include "mods/svc/hook.h"
#include "mods/svc/log.h"
#include "mods/svc/log.hpp"
#include "mods/svc/hook.hpp"
#include "mods/api.h"
#include "d/d_menu_save.h"
#include "types.h"
#include "Z2AudioLib/Z2SeqMgr.h"

ModResult Misc::init()
{
    ModResult result;

    PRE_HOOK(HookCheckBgmIDPlaying, preCheckBgmIDPlaying);

    return MOD_OK;
}

// hack: game over screen waits for the game over music to end
// so pretend it's never actually playing so the retry prompt comes up faster
HookAction Misc::preCheckBgmIDPlaying(ModContext*, void* args, void* retval, void*)
{
    if (false) {  // replace with config check later
        return HOOK_CONTINUE;
    }

    u32 bgmID = mods::arg<u32>(args, 1);
    if (bgmID == Z2BGM_GAME_OVER) {
        *static_cast<bool*>(retval) = false;
        return HOOK_SKIP_ORIGINAL;
    }

    return HOOK_CONTINUE;
}
