#include "LayerSurface.hpp"

#include "../events/Events.hpp"
#include "../hyprpicker.hpp"

CLayerSurface::CLayerSurface(SMonitor* pMonitor) {
    m_pMonitor = pMonitor;

    pSurface = wl_compositor_create_surface(g_pHyprpicker->m_pCompositor);

    if (!pSurface) {
        Debug::log(CRIT, "The compositor did not allow hyprpicker a surface!");
        g_pHyprpicker->finish(1);
        return;
    }

    pLayerSurface = zwlr_layer_shell_v1_get_layer_surface(g_pHyprpicker->m_pLayerShell, pSurface, pMonitor->output, ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, "hyprpicker");

    if (!pLayerSurface) {
        Debug::log(CRIT, "The compositor did not allow hyprpicker a layersurface!");
        g_pHyprpicker->finish(1);
        return;
    }

    zwlr_layer_surface_v1_set_size(pLayerSurface, 0, 0);
    zwlr_layer_surface_v1_set_anchor(
        pLayerSurface, ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);
    zwlr_layer_surface_v1_set_exclusive_zone(pLayerSurface, -1);
    zwlr_layer_surface_v1_set_keyboard_interactivity(pLayerSurface, true);
    zwlr_layer_surface_v1_add_listener(pLayerSurface, &Events::layersurfaceListener, this);
    wl_surface_commit(pSurface);

    wl_display_flush(g_pHyprpicker->m_pWLDisplay);
}

CLayerSurface::~CLayerSurface() {
    wl_surface_destroy(pSurface);
    zwlr_layer_surface_v1_destroy(pLayerSurface);

    if (g_pHyprpicker->m_pWLDisplay)
        wl_display_flush(g_pHyprpicker->m_pWLDisplay);

    g_pHyprpicker->destroyBuffer(&buffers[0]);
    g_pHyprpicker->destroyBuffer(&buffers[1]);
    g_pHyprpicker->destroyBuffer(&screenBuffer);
}