#pragma once

#include "defines.hpp"
#include "helpers/LayerSurface.hpp"
#include "helpers/PoolBuffer.hpp"

enum eOutputMode {
    OUTPUT_CMYK = 0,
    OUTPUT_HEX,
    OUTPUT_RGB,
    OUTPUT_HSL,
    OUTPUT_HSV
};

class CHyprpicker {
  public:
    void                                        init();

    std::mutex                                  m_mtTickMutex;

    wl_compositor*                              m_pCompositor        = nullptr;
    wl_display*                                 m_pWLDisplay         = nullptr;
    wl_registry*                                m_pWLRegistry        = nullptr;
    wl_shm*                                     m_pWLSHM             = nullptr;
    zwlr_layer_shell_v1*                        m_pLayerShell        = nullptr;
    zwlr_screencopy_manager_v1*                 m_pSCMgr             = nullptr;
    wp_cursor_shape_manager_v1*                 m_pCursorShape       = nullptr;
    wp_cursor_shape_device_v1*                  m_pCursorShapeDevice = nullptr;

    xkb_context*                                m_pXKBContext = nullptr;
    xkb_keymap*                                 m_pXKBKeymap  = nullptr;
    xkb_state*                                  m_pXKBState   = nullptr;

    eOutputMode                                 m_bSelectedOutputMode = OUTPUT_HEX;

    bool                                        m_bFancyOutput = true;

    bool                                        m_bAutoCopy       = false;
    bool                                        m_bRenderInactive = false;
    bool                                        m_bNoZoom         = false;

    bool                                        m_bRunning = true;

    std::vector<std::unique_ptr<SMonitor>>      m_vMonitors;
    std::vector<std::unique_ptr<CLayerSurface>> m_vLayerSurfaces;

    void                                        createSeat(wl_seat*);

    CLayerSurface*                              m_pLastSurface;

    Vector2D                                    m_vLastCoords;

    void                                        renderSurface(CLayerSurface*, bool forceInactive = false);

    void                                        createBuffer(SPoolBuffer*, int32_t, int32_t, uint32_t, uint32_t);
    void                                        destroyBuffer(SPoolBuffer*);
    int                                         createPoolFile(size_t, std::string&);
    bool                                        setCloexec(const int&);
    void                                        recheckACK();

    void                                        sendFrame(CLayerSurface*);

    SPoolBuffer*                                getBufferForLS(CLayerSurface*);

    void                                        convertBuffer(SPoolBuffer*);
    void*                                       convert24To32Buffer(SPoolBuffer*);

    void                                        markDirty();

    void                                        finish(int code = 0);

    CColor                                      getColorFromPixel(CLayerSurface*, Vector2D);

  private:
};

inline std::unique_ptr<CHyprpicker> g_pHyprpicker;