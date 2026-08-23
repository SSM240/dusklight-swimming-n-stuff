#include "audio.h"
#include "util.h"
#include "mods/svc/hook.h"
#include "mods/svc/log.h"
#include "mods/svc/log.hpp"
#include "mods/svc/hook.hpp"
#include "mods/api.h"
#include "types.h"
#include "d/actor/d_a_alink.h"
#include "m_Do/m_Do_controller_pad.h"
#include "Z2AudioLib/Z2AudioMgr.h"
#include "Z2AudioLib/Z2SeqMgr.h"
#include "Z2AudioLib/Z2SeMgr.h"

ModResult Audio::init()
{
    ModResult result;

    PRE_HOOK(HookCheckBgmIDPlaying, preCheckBgmIDPlaying);
    PRE_HOOK(HookVoiceStart, preVoiceStart);

    return MOD_OK;
}

ModResult Audio::update()
{
    // music muting
    if (mDoCPd_c::getHoldR(PAD_1) && mDoCPd_c::getTrigDown(PAD_1)) {
        Z2GetAudioMgr()->muteSceneBgm(15, 0.0f);
    }
    else if (mDoCPd_c::getHoldR(PAD_1) && mDoCPd_c::getTrigUp(PAD_1)) {
        Z2GetAudioMgr()->unMuteSceneBgm(15);
    }

    return MOD_OK;
}

// hack: game over screen waits for the game over music to end
// so pretend it's never actually playing so the retry prompt comes up faster
HookAction Audio::preCheckBgmIDPlaying(ModContext*, void* args, void* retval, void*)
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

// stop wolf link panting noises from playing underwater
HookAction Audio::preVoiceStart(ModContext*, void* args, void*, void*)
{
    daAlink_c* player = mods::arg<daAlink_c*>(args, 0);
    u32 i_soundID = mods::arg<int>(args, 1);

    if (player->checkWolf()
        && player->checkModeFlg(daAlink_c::MODE_SWIMMING)
        && !player->checkNoResetFlg0(daAlink_c::FLG0_SWIM_UP))
    {
        if (i_soundID == Z2SE_WL_V_BREATH_WAIT || i_soundID == Z2SE_WL_V_BREATH_WALK) {
            return HOOK_SKIP_ORIGINAL;
        }
    }

    return HOOK_CONTINUE;
}
