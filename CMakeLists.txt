cmake_minimum_required(VERSION 3.26)
project(swimming_n_stuff CXX)

# The Dusklight version to build against: a release tag (e.g. "v1.5.0") or a commit SHA.
if (NOT DUSKLIGHT_VERSION)
    set(DUSKLIGHT_VERSION "7434a0f834866294e7691bd86ab9527011265585") # 2026-08-08
endif ()
# Fetches the Dusklight sources into ./dusklight and provides DUSKLIGHT_DIR.
# Use -DDUSKLIGHT_DIR= to point at an existing Dusklight checkout.
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/FetchDusklight.cmake")
add_subdirectory("${DUSKLIGHT_DIR}/sdk" dusklight-sdk EXCLUDE_FROM_ALL)

add_mod(swimming_n_stuff
        FEATURES game          # remove for service/asset-only mods; add webgpu for GfxService
        SOURCES src/mod.cpp
        MOD_JSON mod.json
        RES_DIR res            # mod resources, including icon.png and banner.png
        # OVERLAY_DIR overlay    # game file overlays; remove if unused
        TEXTURES_DIR textures  # texture replacements; remove if unused
)
