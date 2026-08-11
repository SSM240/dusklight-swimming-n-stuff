#include "airMeter.h"
#include "util.h"
#include <algorithm>
#include "mods/hook.hpp"
#include "mods/svc/hook.h"
#include "mods/svc/hook.hpp"
#include "mods/svc/log.h"
#include "mods/svc/log.hpp"
#include "mods/api.h"
#include "d/actor/d_a_alink.h"
#include "d/actor/d_a_player.h"
#include "d/d_meter2_draw.h"
#include "d/d_com_inf_game.h"
#include "types.h"
#include "JSystem/JAudio2/JAISound.h"

ModResult AirMeter::init() {
    ModResult result;

    PRE_HOOK(HookDrawKanteraScreen, preDrawKanteraScreen);
    POST_HOOK(HookDrawKanteraScreen, postDrawKanteraScreen);
    PRE_HOOK(HookSEStartLevel, preSeStartLevel);

    return MOD_OK;
}

ModResult AirMeter::update() {
    // put in update cus it looks like the game sometimes resets it? maybe?
    // idk for sure but no real reason not to
    // TODO: config
    dComIfGp_setMaxOxygen(1200);

    return MOD_OK;
}

// "kantera" means "lantern" in japanese
// i guess the oil meter came first and the air meter shares code with it

// anyway: dumb hack to change the "low air" criteria without copying the whole 
// function (which ends up being its own whole headache for reasons)
// i miss being able to just modify consts or tiny bits of code like you can in gc modding
HookAction AirMeter::preDrawKanteraScreen(ModContext*, void*, void*, void*) {
    //dMeter2Draw_c* meter = mods::arg<dMeter2Draw_c*>(args, 0);

    prevMaxOxygen = dComIfGp_getMaxOxygen();
    // pretend max oxygen is at most 28 secs for this function
    // so higher values don't make the "low air" sfx play for a super long time (~14 secs max)
    dComIfGp_setMaxOxygen(std::min(prevMaxOxygen, 840));
    return HOOK_CONTINUE;
}

void AirMeter::postDrawKanteraScreen(ModContext*, void*, void*, void*) {
    dComIfGp_setMaxOxygen(prevMaxOxygen);
}

// disable annoying "bloop" sound effect
HookAction AirMeter::preSeStartLevel(ModContext*, void* args, void* retval, void*) {
    JAISoundID soundID = mods::arg<JAISoundID>(args, 1);
    if (soundID == Z2SE_SWIM_TIMER_DEC) { // todo: config?
        *static_cast<bool*>(retval) = false;
        return HOOK_SKIP_ORIGINAL;
    }
    return HOOK_CONTINUE;
}
