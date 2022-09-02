#pragma once

#include "../defines.hpp"
#include "PoolBuffer.hpp"

struct SMonitor;

class CLayerSurface {
public:
    CLayerSurface(SMonitor*);
    ~CLayerSurface();

    SMonitor* m_pMonitor = nullptr;

    zwlr_layer_surface_v1* pLayerSurface = nullptr;
    wl_surface* pSurface = nullptr;
    wl_surface* pCursorSurface = nullptr;

    bool wantsACK = false;
    uint32_t ACKSerial = 0;
    bool working = false;

    int lastBuffer = 0;
    SPoolBuffer buffers[2];
    
    SPoolBuffer screenBuffer;
    uint32_t scflags = 0;

    bool dirty = true;

    bool rendered = false;

    wl_callback* frame_callback = nullptr;

    wl_cursor_image* pCursorImg = nullptr;
};