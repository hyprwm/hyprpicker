#pragma once

#include "../defines.hpp"
#include <hyprutils/math/Vector2D.hpp>
using namespace Hyprutils::Math;

struct SMonitor {
    std::string               name         = "";
    wl_output*                output       = nullptr;
    uint32_t                  wayland_name = 0;
    Vector2D                  size;
    int                       scale;
    wl_output_transform       transform = WL_OUTPUT_TRANSFORM_NORMAL;

    bool                      ready = false;

    zwlr_screencopy_frame_v1* pSCFrame = nullptr;
};