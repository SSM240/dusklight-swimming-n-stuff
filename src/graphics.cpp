#include "graphics.h"
#include "util.h"
#include "mods/svc/hook.h"
#include "mods/svc/log.h"
#include "mods/svc/log.hpp"
#include "mods/svc/hook.hpp"
#include "mods/api.h"
#include "types.h"
#include "d/actor/d_a_alink.h"
#include "m_Do/m_Do_controller_pad.h"

ModResult Graphics::init()
{
    ModResult result;

    POST_HOOK(HookSetLightPalnoGet, postSetLightPalnoGet);
    POST_HOOK(HookCalcColorSet, postCalcColorSet);

    return MOD_OK;
}

ModResult Graphics::update()
{
    

    return MOD_OK;
}

void Graphics::postSetLightPalnoGet(ModContext*, void* args, void*, void*) {
    // crying and pissing
    u8* prev_pal_start_id_p = mods::arg_ref<u8*>(args, 4);
    u8* prev_pal_end_id_p = mods::arg_ref<u8*>(args, 5);
    u8* next_pal_start_id_p = mods::arg_ref<u8*>(args, 6);
    u8* next_pal_end_id_p = mods::arg_ref<u8*>(args, 7);

    return; // disable this stuff for now

    if (mDoCPd_c::getTrigR(PAD_1)) {
        //mods::log::info("{}, {}, {}, {}", *prev_pal_start_id_p, *prev_pal_end_id_p,
        //    *next_pal_start_id_p, *next_pal_end_id_p);
        //*prev_pal_start_id_p = 5;
        //*prev_pal_end_id_p = 5;
        //*next_pal_start_id_p = 5;
        //*next_pal_end_id_p = 5;
        stage_palette_info_class* next_pal_start_p =
            &g_env_light.stage_palette_info[*next_pal_start_id_p];
        for (int i = 0; i < 4; i++) {
            //mods::log::info("[{}]: {} {} {}", i, next_pal_start_p->bg_amb_col[i].r,
            //    next_pal_start_p->bg_amb_col[i].g, next_pal_start_p->bg_amb_col[i].b);
        }
        //next_pal_start_p->bg_amb_col[0].r = 0;
        //next_pal_start_p->bg_amb_col[0].g = 128;
        //next_pal_start_p->bg_amb_col[0].b = 255;
        mods::log::info("color: {} {} {}", next_pal_start_p->fog_col.r,
            next_pal_start_p->fog_col.g, next_pal_start_p->fog_col.b);
        mods::log::info("near: {}", (f32)next_pal_start_p->fog_start_z);
        mods::log::info("far: {}", (f32)next_pal_start_p->fog_end_z);

        next_pal_start_p->fog_col.r = 50;
        next_pal_start_p->fog_col.g = 90;
        next_pal_start_p->fog_col.b = 190;
        next_pal_start_p->fog_start_z = BE<f32>(-5000.0f);
        next_pal_start_p->fog_end_z = BE<f32>(10000.0f);
        // 0: main ambient color
        // 1: underwater fog colors? at least in lake hylia
        // 2: sunlight color? seems to affect waterfalls and sunbeams
        // 3: unclear but seems to be often unused
    }
}

void Graphics::postCalcColorSet(ModContext*, void* args, void*, void*) {
    GXColorS10* out_color_p = mods::arg_ref<GXColorS10*>(args, 0);
    if (mDoCPd_c::getHoldR(PAD_1) && out_color_p == &dKy_getEnvlight()->bg_amb_col[0]) {
        //dKy_getEnvlight()->bg_amb_col[0].r = 150;
        // i think this is getting overwritten somewhere
    }

    //return HOOK_CONTINUE;
}
