#pragma once

#include "mods/api.h"
#include "mods/svc/config.h"
#include "types.h"

namespace Config
{

ModResult init();

bool getWolfLinkSwimming();
s32 getMaxOxygen();
bool getUseYToSwim();
bool getKeepCameraOnDeath();
bool getFasterGameOver();

}
