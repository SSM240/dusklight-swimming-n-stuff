#include "camera.h"
#include "util.h"
#include "mods/svc/hook.h"
#include "mods/svc/log.h"
#include "mods/svc/log.hpp"
#include "mods/svc/hook.hpp"
#include "mods/api.h"
#include "d/actor/d_a_alink.h"
#include "d/actor/d_a_player.h"
#include "d/d_com_inf_game.h"
#include "types.h"

ModResult Camera::init()
{
    ModResult result;

    POST_HOOK(HookProcCoDeadInit, postLinkProcCoDeadInit);

    return MOD_OK;
}

void Camera::postLinkProcCoDeadInit(ModContext*, void* args, void*, void*) {
    daAlink_c* player = mods::arg<daAlink_c*>(args, 0);
    // undo camera change on death
    // todo: config
    player->mDemo.resetDemoType();
}
