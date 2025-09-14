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

    SP<CCWlCompositor>                          m_pCompositor;
    SP<CCWlRegistry>                            m_pRegistry;
    SP<CCWlShm>                                 m_pSHM;
    SP<CCZwlrLayerShellV1>                      m_pLayerShell;
    SP<CCZwlrScreencopyManagerV1>               m_pScreencopyMgr;
    SP<CCWpCursorShapeManagerV1>                m_pCursorShapeMgr;
    SP<CCWpCursorShapeDeviceV1>                 m_pCursorShapeDevice;
    SP<CCWlSeat>                                m_pSeat;
    SP<CCWlKeyboard>                            m_pKeyboard;
    SP<CCWlPointer>                             m_pPointer;
    SP<CCWpFractionalScaleManagerV1>            m_pFractionalMgr;
    SP<CCWpViewporter>                          m_pViewporter;
    wl_display*                                 m_pWLDisplay = nullptr;

    xkb_context*                                m_pXKBContext = nullptr;
    xkb_keymap*                                 m_pXKBKeymap  = nullptr;
    xkb_state*                                  m_pXKBState   = nullptr;

    eOutputMode                                 m_bSelectedOutputMode = OUTPUT_HEX;

    bool                                        m_bFancyOutput = true;

    bool                                        m_bAutoCopy       = false;
    bool                                        m_bNotify         = false;
    bool                                        m_bRenderInactive = false;
    bool                                        m_bNoZoom         = false;
    bool                                        m_bNoFractional   = false;
    bool                                        m_bDisablePreview = false;
    bool                                        m_bUseLowerCase   = false;

    bool                                        m_bRunning   = true;
    float                                       m_fZoomScale = 10.0;

    std::vector<std::unique_ptr<SMonitor>>      m_vMonitors;
    std::vector<std::unique_ptr<CLayerSurface>> m_vLayerSurfaces;

    CLayerSurface*                              m_pLastSurface;

    Vector2D                                    m_vLastCoords;
    bool                                        m_bCoordsInitialized = false;

    void                                        renderSurface(CLayerSurface*, bool forceInactive = false);

    int                                         createPoolFile(size_t, std::string&);
    bool                                        setCloexec(const int&);
    void                                        recheckACK();
    void                                        initKeyboard();
    void                                        initMouse();

    SP<SPoolBuffer>                             getBufferForLS(CLayerSurface*);

    void                                        convertBuffer(SP<SPoolBuffer>);
    void*                                       convert24To32Buffer(SP<SPoolBuffer>);

    void                                        markDirty();

    void                                        finish(int code = 0);

    CColor                                      getColorFromPixel(CLayerSurface*, Vector2D);

  private:
};

inline std::unique_ptr<CHyprpicker> g_pHyprpicker;
