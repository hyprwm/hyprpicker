#pragma once

#include "../defines.hpp"

struct SMonitor {
    std::string name = "";
    wl_output* output = nullptr;
    uint32_t wayland_name = 0;
    Vector2D size;
    int scale;

    bool ready = false;

    zwlr_screencopy_frame_v1* pSCFrame = nullptr;
};