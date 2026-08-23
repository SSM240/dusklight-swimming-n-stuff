cmake_minimum_required(VERSION 3.26)
project(swimming_n_stuff CXX)

# The Dusklight version to build against: a release tag (e.g. "v1.5.0") or a commit SHA.
if (NOT DUSKLIGHT_VERSION)
    set(DUSKLIGHT_VERSION "41d5148793cf67f0e93e79cf3eab8718d6e9849d") # 2026-08-22
endif ()
# Fetches the Dusklight sources into ./dusklight and provides DUSKLIGHT_DIR.
# Use -DDUSKLIGHT_DIR= to point at an existing Dusklight checkout.
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/FetchDusklight.cmake")
add_subdirectory("${DUSKLIGHT_DIR}/sdk" dusklight-sdk EXCLUDE_FROM_ALL)

add_mod(swimming_n_stuff
        FEATURES game fmt      # remove for service/asset-only mods; add webgpu for GfxService
        SOURCES 
            src/mod.cpp
            src/airMeter.cpp
            src/bubbles.cpp
            src/camera.cpp
            src/misc.cpp
            src/wolfLinkSwimming.cpp
        MOD_JSON mod.json
        RES_DIR res            # mod resources, including icon.png and banner.png
        # OVERLAY_DIR overlay    # game file overlays; remove if unused
        TEXTURES_DIR textures  # texture replacements; remove if unused
)
