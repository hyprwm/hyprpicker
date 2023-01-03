#include "Events.hpp"
#include "../hyprpicker.hpp"

void Events::geometry(void *data, wl_output *output, int32_t x, int32_t y, int32_t width_mm, int32_t height_mm, int32_t subpixel, const char *make, const char *model, int32_t transform) {
    // ignored
}

void Events::mode(void *data, wl_output *output, uint32_t flags, int32_t width, int32_t height, int32_t refresh) {
    // ignored
}

void Events::done(void *data, wl_output *wl_output) {
    const auto PMONITOR = (SMonitor*)data;

    PMONITOR->ready = true;
}

void Events::scale(void *data, wl_output *wl_output, int32_t scale) {
    const auto PMONITOR = (SMonitor*)data;

    PMONITOR->scale = scale;
}

void Events::name(void *data, wl_output *wl_output, const char *name) {
    const auto PMONITOR = (SMonitor*)data;

    if (name)
        PMONITOR->name = name;
}

void Events::description(void *data, wl_output *wl_output, const char *description) {
    // i do not care
}

void Events::ls_configure(void *data, zwlr_layer_surface_v1 *surface, uint32_t serial, uint32_t width, uint32_t height) {
    const auto PLAYERSURFACE = (CLayerSurface *)data;

    PLAYERSURFACE->m_pMonitor->size = Vector2D(width, height);
    PLAYERSURFACE->ACKSerial = serial;
    PLAYERSURFACE->wantsACK = true;
    PLAYERSURFACE->working = true;

    g_pHyprpicker->recheckACK();
}

void Events::handleGlobal(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        g_pHyprpicker->m_pCompositor = (wl_compositor *)wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        g_pHyprpicker->m_pWLSHM = (wl_shm *)wl_registry_bind(registry, name, &wl_shm_interface, 1);
    } else if (strcmp(interface, wl_output_interface.name) == 0) {
        g_pHyprpicker->m_mtTickMutex.lock();

        const auto PMONITOR = g_pHyprpicker->m_vMonitors.emplace_back(std::make_unique<SMonitor>()).get();
        PMONITOR->wayland_name = name;
        PMONITOR->name = "";
        PMONITOR->output = (wl_output *)wl_registry_bind(registry, name, &wl_output_interface, 4);
        wl_output_add_listener(PMONITOR->output, &Events::outputListener, PMONITOR);

        g_pHyprpicker->m_mtTickMutex.unlock();
    } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
        g_pHyprpicker->m_pLayerShell = (zwlr_layer_shell_v1 *)wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, 1);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        g_pHyprpicker->createSeat((wl_seat*)wl_registry_bind(registry, name, &wl_seat_interface, 1));
    } else if (strcmp(interface, zwlr_screencopy_manager_v1_interface.name) == 0) {
        g_pHyprpicker->m_pSCMgr = (zwlr_screencopy_manager_v1*)wl_registry_bind(registry, name, &zwlr_screencopy_manager_v1_interface, 1);
    }
}

void Events::handleGlobalRemove(void *data, struct wl_registry *registry, uint32_t name) {
    // todo
}

void Events::handleCapabilities(void *data, wl_seat *wl_seat, uint32_t capabilities) {
    if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
        wl_pointer_add_listener(wl_seat_get_pointer(wl_seat), &pointerListener, wl_seat);
    } else {
        Debug::log(CRIT, "Hyprpicker cannot work without a pointer!");
        g_pHyprpicker->finish(1);
    }

    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
        wl_keyboard_add_listener(wl_seat_get_keyboard(wl_seat), &keyboardListener, wl_seat);
    }
}

void Events::handlePointerEnter(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
    g_pHyprpicker->markDirty();

    for (auto& ls : g_pHyprpicker->m_vLayerSurfaces) {
        if (ls->pSurface == surface) {
            g_pHyprpicker->m_pLastSurface = ls.get();

            wl_surface_set_buffer_scale(ls->pCursorSurface, ls->m_pMonitor->scale);
            wl_surface_attach(ls->pCursorSurface, wl_cursor_image_get_buffer(ls->pCursorImg), 0, 0);
            wl_pointer_set_cursor(wl_pointer, serial, ls->pCursorSurface, ls->pCursorImg->hotspot_x / ls->m_pMonitor->scale, ls->pCursorImg->hotspot_y / ls->m_pMonitor->scale);
            wl_surface_commit(ls->pCursorSurface);
        }
    }
}

void Events::handlePointerLeave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface) {
    for (auto& ls : g_pHyprpicker->m_vLayerSurfaces) {
        if (ls->pSurface == surface) {
            g_pHyprpicker->renderSurface(ls.get(), true);
        }
    }
}

void Events::handlePointerAxis(void *data, wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {
    // ignored
}

void Events::handlePointerMotion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {

    auto x = wl_fixed_to_double(surface_x);
    auto y = wl_fixed_to_double(surface_y);

    g_pHyprpicker->m_vLastCoords = {x, y};

    g_pHyprpicker->markDirty();
}

void Events::handlePointerButton(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t button_state) {
    auto fmax3 = [](float a, float b, float c) -> float {
        return (a > b && a > c) ? a : (b > c) ? b : c;
    };
    auto fmin3 = [](float a, float b, float c) -> float {
        return (a < b && a < c) ? a : (b < c) ? b : c;
    };

    // get the px and print it
    const auto SCALE = Vector2D{
        g_pHyprpicker->m_pLastSurface->screenBuffer.pixelSize.x / (g_pHyprpicker->m_pLastSurface->buffers[0].pixelSize.x / g_pHyprpicker->m_pLastSurface->m_pMonitor->scale),
        g_pHyprpicker->m_pLastSurface->screenBuffer.pixelSize.y / (g_pHyprpicker->m_pLastSurface->buffers[0].pixelSize.y / g_pHyprpicker->m_pLastSurface->m_pMonitor->scale)
    };

    const auto CLICKPOS = Vector2D{g_pHyprpicker->m_vLastCoords.floor().x * SCALE.x, g_pHyprpicker->m_vLastCoords.floor().y * SCALE.y};

    const auto COL = g_pHyprpicker->getColorFromPixel(g_pHyprpicker->m_pLastSurface, CLICKPOS);

    switch (g_pHyprpicker->m_bSelectedOutputMode) {
        case OUTPUT_CMYK:
        {
            // http://www.codeproject.com/KB/applications/xcmyk.aspx

            float r = 1 - COL.r / 255.0f,
                  g = 1 - COL.g / 255.0f,
                  b = 1 - COL.b / 255.0f;
            float k = fmin3(r, g, b),
                  K = 1 - k;
            float c = (r - k) / K,
                  m = (g - k) / K,
                  y = (b - k) / K;

            c = std::round(c * 100);
            m = std::round(m * 100);
            y = std::round(y * 100);
            k = std::round(k * 100);

            if (g_pHyprpicker->m_bFancyOutput)
                Debug::log(NONE, "\033[38;2;%i;%i;%im%g%% %g%% %g%% %g%%\033[0m", COL.r, COL.g, COL.b, c, m, y, k);
            else
                Debug::log(NONE, "%g%% %g%% %g%% %g%%", c, m, y, k);

            if (!g_pHyprpicker->m_bAutoCopy)
                g_pHyprpicker->finish();
            Clipboard::copy("%g%% %g%% %g%% %g%%", c, m, y, k);
            break;
        }
        case OUTPUT_HEX:
        {
            auto toHex = [](int i) -> std::string {
                const char* DS = "0123456789ABCDEF";

                std::string result = "";

                result += DS[i / 16];
                result += DS[i % 16];

                return result;
            };

            if (g_pHyprpicker->m_bFancyOutput)
                Debug::log(NONE, "\033[38;2;%i;%i;%im#%s%s%s\033[0m", COL.r, COL.g, COL.b, toHex(COL.r).c_str(), toHex(COL.g).c_str(), toHex(COL.b).c_str());
            else
                Debug::log(NONE, "#%s%s%s", toHex(COL.r).c_str(), toHex(COL.g).c_str(), toHex(COL.b).c_str());

            if (!g_pHyprpicker->m_bAutoCopy)
                g_pHyprpicker->finish();
            Clipboard::copy("#%s%s%s", toHex(COL.r).c_str(), toHex(COL.g).c_str(), toHex(COL.b).c_str());
            break;
        }
        case OUTPUT_RGB:
        {
            if (g_pHyprpicker->m_bFancyOutput)
                Debug::log(NONE, "\033[38;2;%i;%i;%im%i %i %i\033[0m", COL.r, COL.g, COL.b, COL.r, COL.g, COL.b);
            else
                Debug::log(NONE, "%i %i %i", COL.r, COL.g, COL.b);

            if (!g_pHyprpicker->m_bAutoCopy)
                g_pHyprpicker->finish();
            Clipboard::copy("%i %i %i", COL.r, COL.g, COL.b);
            break;
        }
        case OUTPUT_HSL:
        case OUTPUT_HSV:
        {
            // https://en.wikipedia.org/wiki/HSL_and_HSV#From_RGB

            auto floatEq = [](float a, float b) -> bool {
              return std::nextafter(a, std::numeric_limits<double>::lowest()) <= b && std::nextafter(a, std::numeric_limits<double>::max()) >= b;
            };

            float h, s, l, v;
            float r = COL.r / 255.0f,
                  g = COL.g / 255.0f,
                  b = COL.b / 255.0f;
            float max = fmax3(r, g, b),
                  min = fmin3(r, g, b);
            float c = max - min;

            v = max;
            if (c == 0)
                h = 0;
            else if (v == r)
                h = 60 * (0 + (g - b) / c);
            else if (v == g)
                h = 60 * (2 + (b - r) / c);
            else /* v == b */
                h = 60 * (4 + (r - g) / c);

            float l_or_v;
            if (g_pHyprpicker->m_bSelectedOutputMode == OUTPUT_HSL) {
                l = (max + min) / 2;
                s = (floatEq(l, 0.0f) || floatEq(l, 1.0f)) ? 0 : (v - l) / std::min(l, 1 - l);
                l_or_v = std::round(l * 100);
            } else {
                v = max;
                s = floatEq(v, 0.0f) ? 0 : c / v;
                l_or_v = std::round(v * 100);
            }

            h = std::round(h);
            s = std::round(s * 100);

            if (g_pHyprpicker->m_bFancyOutput)
                Debug::log(NONE, "\033[38;2;%i;%i;%im%g %g%% %g%%\033[0m", COL.r, COL.g, COL.b, h, s, l_or_v);
            else
                Debug::log(NONE, "%g %g%% %g%%", h, s, l_or_v);

            if (!g_pHyprpicker->m_bAutoCopy)
                g_pHyprpicker->finish();
            Clipboard::copy("%g %g%% %g%%", h, s, l_or_v);
            break;
        }
    }

    if (g_pHyprpicker->m_bAutoCopy)
        g_pHyprpicker->finish();
}

void Events::handleKeyboardKeymap(void* data, wl_keyboard* wl_keyboard, uint format, int fd, uint size) {

}

void Events::handleKeyboardKey(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    if (key == 1) // escape
        g_pHyprpicker->finish();
}

void Events::handleKeyboardEnter(void* data, wl_keyboard* wl_keyboard, uint serial, wl_surface* surface, wl_array* keys) {

}

void Events::handleKeyboardLeave(void* data, wl_keyboard* wl_keyboard, uint serial, wl_surface* surface) {

}

void Events::handleKeyboardModifiers(void* data, wl_keyboard* wl_keyboard, uint serial, uint mods_depressed, uint mods_latched, uint mods_locked, uint group) {

}


void Events::handleFrameDone(void *data, struct wl_callback *callback, uint32_t time) {
    CLayerSurface* pLS = (CLayerSurface*)data;

    if (pLS->frame_callback)
        wl_callback_destroy(pLS->frame_callback);

    pLS->frame_callback = nullptr;

    if (pLS->dirty || !pLS->rendered)
        g_pHyprpicker->renderSurface(g_pHyprpicker->m_pLastSurface);
}

void Events::handleBufferRelease(void *data, struct wl_buffer *wl_buffer) {
    auto buf = (SPoolBuffer*)data;
    buf->busy = false;
}

void Events::handleSCBuffer(void *data, struct zwlr_screencopy_frame_v1 *frame, uint32_t format, uint32_t width, uint32_t height, uint32_t stride) {
    const auto PLS = (CLayerSurface*)data;

    g_pHyprpicker->createBuffer(&PLS->screenBuffer, width, height, format);

    zwlr_screencopy_frame_v1_copy(frame, PLS->screenBuffer.buffer);
}

void Events::handleSCFlags(void *data, struct zwlr_screencopy_frame_v1 *frame, uint32_t flags) {
    const auto PLS = (CLayerSurface *)data;

    PLS->scflags = flags;

    g_pHyprpicker->recheckACK();

    g_pHyprpicker->renderSurface(PLS);
}

void Events::handleSCReady(void *data, struct zwlr_screencopy_frame_v1 *frame, uint32_t tv_sec_hi, uint32_t tv_sec_lo, uint32_t tv_nsec) {
    // ignore
}

void Events::handleSCFailed(void *data, struct zwlr_screencopy_frame_v1 *frame) {
    Debug::log(CRIT, "Failed to get a Screencopy!");
    g_pHyprpicker->finish(1);
}
