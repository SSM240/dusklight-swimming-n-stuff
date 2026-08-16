#include "camera.h"
#include "util.h"
#include "mods/svc/hook.h"
#include "mods/svc/log.h"
#include "mods/svc/log.hpp"
#include "mods/svc/hook.hpp"
#include "mods/api.h"
#include "d/actor/d_a_alink.h"
#include "d/actor/d_a_player.h"
#include "d/d_camera.h"
#include "d/d_com_inf_game.h"
#include "types.h"

ModResult Camera::init()
{
    ModResult result;

    POST_HOOK(HookProcCoDead, postLinkProcCoDead);
    POST_HOOK(HookCheckForceLockTarget, postCheckForceLockTarget);

    return MOD_OK;
}

void Camera::postLinkProcCoDead(ModContext*, void* args, void*, void*) {
    daAlink_c* player = mods::arg<daAlink_c*>(args, 0);
    // prevent camera from changing on death
    // todo: config
    player->mProcVar3.field_0x300e = 0;
    forceAllowCameraControl = true;
}

// very stupid hack
// dCamera_c::checkForceLockTarget is a function that only runs during dCamera_c::Run
// and conveniently, it is run just after it zeroes out the c-stick values when a 
// cutscene is running
// so we can hijack the function and run updatePad to read the c-stick values again
void Camera::postCheckForceLockTarget(ModContext*, void* args, void*, void*) {
    dCamera_c* camera = mods::arg<dCamera_c*>(args, 0);
    if (forceAllowCameraControl) {
        camera->updatePad();
    }
    forceAllowCameraControl = false;
}
