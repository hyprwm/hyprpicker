#pragma once

#include "../defines.hpp"
#include <hyprutils/math/Vector2D.hpp>
using namespace Hyprutils::Math;

class CLayerSurface;

struct SMonitor {
    SMonitor(SP<CCWlOutput> output_);
    void                        initSCFrame();

    std::string                 name         = "";
    SP<CCWlOutput>              output       = nullptr;
    uint32_t                    wayland_name = 0;
    Vector2D                    size;
    int32_t                     scale;
    wl_output_transform         transform = WL_OUTPUT_TRANSFORM_NORMAL;

    bool                        ready = false;

    CLayerSurface*              pLS      = nullptr;
    SP<CCZwlrScreencopyFrameV1> pSCFrame = nullptr;
};
