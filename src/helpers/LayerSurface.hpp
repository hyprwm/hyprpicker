#pragma once

#include "../defines.hpp"
#include "PoolBuffer.hpp"

struct SMonitor;

class CLayerSurface {
  public:
    CLayerSurface(SMonitor*);
    ~CLayerSurface();

    void                      sendFrame();
    void                      markDirty();

    SMonitor*                 m_pMonitor = nullptr;

    SP<CCZwlrLayerSurfaceV1>  pLayerSurface    = nullptr;
    SP<CCWlSurface>           pSurface         = nullptr;
    SP<CCWpViewport>          pViewport        = nullptr;
    SP<CCWpFractionalScaleV1> pFractionalScale = nullptr;

    float                     fractionalScale = 1.F;
    bool                      wantsACK        = false;
    bool                      wantsReload     = false;
    uint32_t                  ACKSerial       = 0;
    bool                      working         = false;

    int                       lastBuffer = 0;
    SP<SPoolBuffer>           buffers[2];

    SP<SPoolBuffer>           screenBuffer;
    uint32_t                  scflags            = 0;
    uint32_t                  screenBufferFormat = 0;

    bool                      dirty = true;

    bool                      rendered = false;

    SP<CCWlCallback>          frameCallback = nullptr;
    bool                      forceRerender = false;
};