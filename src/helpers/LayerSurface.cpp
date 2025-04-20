#include "LayerSurface.hpp"

#include "../hyprpicker.hpp"

CLayerSurface::CLayerSurface(SMonitor* pMonitor) : m_pMonitor(pMonitor) {
    pSurface = makeShared<CCWlSurface>(g_pHyprpicker->m_pCompositor->sendCreateSurface());

    if (!pSurface) {
        Debug::log(CRIT, "The compositor did not allow hyprpicker a surface!");
        g_pHyprpicker->finish(1);
        return;
    }

    if (!g_pHyprpicker->m_bNoFractional) {
        pViewport = makeShared<CCWpViewport>(g_pHyprpicker->m_pViewporter->sendGetViewport(pSurface->resource()));

        // this will not actually be used, as we assume we'll be fullscreen and we can get the real dimensions from screencopy, but we'll have
        // this for if we need it in the future
        pFractionalScale = makeShared<CCWpFractionalScaleV1>(g_pHyprpicker->m_pFractionalMgr->sendGetFractionalScale(pSurface->resource()));
        pFractionalScale->setPreferredScale([this](CCWpFractionalScaleV1* r, uint32_t scale120) { //
            Debug::log(TRACE, "Received a preferredScale for %s: %.2f", m_pMonitor->name.c_str(), scale120 / 120.F);
            fractionalScale = scale120 / 120.F;
            wantsReload     = true;
            g_pHyprpicker->recheckACK();
        });
    }

    pLayerSurface = makeShared<CCZwlrLayerSurfaceV1>(
        g_pHyprpicker->m_pLayerShell->sendGetLayerSurface(pSurface->resource(), pMonitor->output->resource(), ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, "hyprpicker"));

    if (!pLayerSurface) {
        Debug::log(CRIT, "The compositor did not allow hyprpicker a layersurface!");
        g_pHyprpicker->finish(1);
        return;
    }

    pLayerSurface->setConfigure([this](CCZwlrLayerSurfaceV1* r, uint32_t serial, uint32_t width, uint32_t height) {
        m_pMonitor->size = {(double)width, (double)height};
        ACKSerial        = serial;
        wantsACK         = true;
        working          = true;

        g_pHyprpicker->recheckACK();
    });

    pLayerSurface->sendSetSize(0, 0);
    pLayerSurface->sendSetAnchor((zwlrLayerSurfaceV1Anchor)(ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
                                                            ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT));
    pLayerSurface->sendSetExclusiveZone(-1);
    pLayerSurface->sendSetKeyboardInteractivity(1);
    pSurface->sendCommit();

    wl_display_flush(g_pHyprpicker->m_pWLDisplay);
}

CLayerSurface::~CLayerSurface() {
    pLayerSurface.reset();
    pSurface.reset();
    frameCallback.reset();

    if (g_pHyprpicker->m_pWLDisplay)
        wl_display_flush(g_pHyprpicker->m_pWLDisplay);
}

// this has to be a separate function because frameCallback.reset() will destroy the listener func
static void onCallbackDone(CLayerSurface* surf, uint32_t when) {
    surf->frameCallback.reset();

    g_pHyprpicker->renderSurface(surf);

    surf->forceRerender = false;
}

void CLayerSurface::sendFrame() {
    lastBuffer          = lastBuffer == 0 ? 1 : 0;
    const auto& PBUFFER = buffers[lastBuffer];

    frameCallback = makeShared<CCWlCallback>(pSurface->sendFrame());
    frameCallback->setDone([this](CCWlCallback* r, uint32_t when) { onCallbackDone(this, when); });

    pSurface->sendDamageBuffer(0, 0, 0xFFFF, 0xFFFF);

    pSurface->sendAttach(PBUFFER->buffer.get(), 0, 0);
    if (!g_pHyprpicker->m_bNoFractional) {
        pSurface->sendSetBufferScale(1);
        pViewport->sendSetDestination(m_pMonitor->size.x, m_pMonitor->size.y);
    } else
        pSurface->sendSetBufferScale(m_pMonitor->scale);

    pSurface->sendCommit();

    dirty = false;
}

void CLayerSurface::markDirty() {
    frameCallback = makeShared<CCWlCallback>(pSurface->sendFrame());
    frameCallback->setDone([this](CCWlCallback* r, uint32_t when) { onCallbackDone(this, when); });

    dirty = true;
}
