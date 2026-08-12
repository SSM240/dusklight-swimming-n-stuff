#pragma once
#include "mods/api.h"
#include "mods/svc/log.hpp"

#define INSTALL_HOOK(entry, func, type)              \
mods::log::debug("installing {}...", #func);         \
result = mods::hook_##type<entry>(func);             \
if (result != MOD_OK) {                              \
    mods::log::error("failed to install {}", #func); \
    return result;                                   \
}

// installs a pre-hook with logging
// returns early and logs an error when it fails
#define PRE_HOOK(entry, func) INSTALL_HOOK(entry, func, add_pre)
// installs a post-hook with logging
// returns early and logs an error when it fails
#define POST_HOOK(entry, func) INSTALL_HOOK(entry, func, add_post)
// installs a replace-hook with logging
// returns early and logs an error when it fails
#define REPLACE_HOOK(entry, func) INSTALL_HOOK(entry, func, replace)

// calls the given namespace's init function
// returns early and logs an error when it fails
#define INIT(name)                                \
result = name::init();                            \
if (result != MOD_OK) {                           \
    mods::log::error("failed to init {}", #name); \
    return result;                                \
}

namespace Util {

}
