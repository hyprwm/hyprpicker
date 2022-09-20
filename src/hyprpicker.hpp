#pragma once

#include "defines.hpp"
#include "helpers/LayerSurface.hpp"
#include "helpers/PoolBuffer.hpp"

enum eOutputMode {
    OUTPUT_HEX = 0,
    OUTPUT_RGB
};

class CHyprpicker {
public:
    void            init();

    std::mutex  m_mtTickMutex;

    wl_compositor*  m_pCompositor;
    wl_display*     m_pWLDisplay;
    wl_registry*    m_pWLRegistry;
    wl_shm*         m_pWLSHM;
    zwlr_layer_shell_v1* m_pLayerShell;
    zwlr_screencopy_manager_v1* m_pSCMgr;

    eOutputMode     m_bSelectedOutputMode = OUTPUT_HEX;

    bool            m_bAutoCopy = false;
    bool            m_bFancyOutput = true;

    bool            m_bRunning = true;

    std::vector<std::unique_ptr<SMonitor>> m_vMonitors;
    std::vector<std::unique_ptr<CLayerSurface>> m_vLayerSurfaces;

    void            createSeat(wl_seat*);

    CLayerSurface*  m_pLastSurface;

    Vector2D        m_vLastCoords;

    void            renderSurface(CLayerSurface*, bool forceInactive = false);

    void            createBuffer(SPoolBuffer*, int32_t, int32_t, uint32_t);
    void            destroyBuffer(SPoolBuffer*);
    int             createPoolFile(size_t, std::string&);
    bool            setCloexec(const int&);

    void            recheckACK();

    void            sendFrame(CLayerSurface*);

    SPoolBuffer*    getBufferForLS(CLayerSurface*);

    void            convertBuffer(SPoolBuffer*);

    void            markDirty();

    void            finish(int code = 0);

    CColor          getColorFromPixel(CLayerSurface*, Vector2D);
private:
    
};

inline std::unique_ptr<CHyprpicker> g_pHyprpicker;