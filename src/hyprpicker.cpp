#include "hyprpicker.hpp"
#include "events/Events.hpp"

void CHyprpicker::init() {
    m_pWLDisplay = wl_display_connect(nullptr);

    if (!m_pWLDisplay) {
        Debug::log(CRIT, "No wayland compositor running!");
        exit(1);
        return;
    }

    m_pWLRegistry = wl_display_get_registry(m_pWLDisplay);

    wl_registry_add_listener(m_pWLRegistry, &Events::registryListener, nullptr);

    wl_display_roundtrip(m_pWLDisplay);

    for (auto& m : m_vMonitors) {
        m_vLayerSurfaces.emplace_back(std::make_unique<CLayerSurface>(m.get()));

        m_pLastSurface = m_vLayerSurfaces.back().get();

        m->pSCFrame = zwlr_screencopy_manager_v1_capture_output(m_pSCMgr, false, m->output);

        zwlr_screencopy_frame_v1_add_listener(m->pSCFrame, &Events::screencopyListener, m_pLastSurface);

        m_pLastSurface->pCursorSurface = wl_compositor_create_surface(m_pCompositor);
    }

    wl_display_roundtrip(m_pWLDisplay);

    while (m_bRunning && wl_display_dispatch(m_pWLDisplay) != -1) {
        //renderSurface(m_pLastSurface);
    }
}

void CHyprpicker::finish(int code) {
    for (auto& ls : m_vLayerSurfaces) {
        destroyBuffer(&ls->buffers[0]);
        destroyBuffer(&ls->buffers[1]);
        destroyBuffer(&ls->screenBuffer);
    }

    exit(code);
}

void CHyprpicker::recheckACK() {
    for (auto& ls : m_vLayerSurfaces) {
        if (ls->wantsACK && ls->screenBuffer.buffer) {
            ls->wantsACK = false;
            zwlr_layer_surface_v1_ack_configure(ls->pLayerSurface, ls->ACKSerial);

            if (!ls->buffers[0].buffer) {
                createBuffer(&ls->buffers[0], ls->m_pMonitor->size.x * ls->m_pMonitor->scale, ls->m_pMonitor->size.y * ls->m_pMonitor->scale, WL_SHM_FORMAT_ARGB8888);
                createBuffer(&ls->buffers[1], ls->m_pMonitor->size.x * ls->m_pMonitor->scale, ls->m_pMonitor->size.y * ls->m_pMonitor->scale, WL_SHM_FORMAT_ARGB8888);

                int XCURSOR_SIZE = 24;
                if (getenv("XCURSOR_SIZE")) {
                    XCURSOR_SIZE = std::stoi(getenv("XCURSOR_SIZE"));
                }

                ls->pCursorImg = wl_cursor_theme_get_cursor(wl_cursor_theme_load(getenv("XCURSOR_THEME"), XCURSOR_SIZE * ls->m_pMonitor->scale, m_pWLSHM), "left_ptr")->images[0];
            }
        }
    }

    markDirty();
}

void CHyprpicker::markDirty() {
    for (auto& ls : m_vLayerSurfaces) {
        if (ls->frame_callback)
            continue;

        ls->frame_callback = wl_surface_frame(ls->pSurface);
        wl_callback_add_listener(ls->frame_callback, &Events::frameListener, ls.get());
        wl_surface_commit(ls->pSurface);

        ls->dirty = true;
    }
}

SPoolBuffer* CHyprpicker::getBufferForLS(CLayerSurface* pLS) {
    SPoolBuffer* returns = nullptr;

    for (auto i = 0; i < 2; ++i) {
        if (pLS->buffers[i].busy)
            continue;

        returns = &pLS->buffers[i];
    }

    if (!returns)
        return nullptr;

    returns->busy = true;

    return returns;
}

bool CHyprpicker::setCloexec(const int& FD) {
    long flags = fcntl(FD, F_GETFD);
    if (flags == -1) {
        return false;
    }

    if (fcntl(FD, F_SETFD, flags | FD_CLOEXEC) == -1) {
        return false;
    }

    return true;
}

int CHyprpicker::createPoolFile(size_t size, std::string& name) {
    const auto XDGRUNTIMEDIR = getenv("XDG_RUNTIME_DIR");
    if (!XDGRUNTIMEDIR) {
        Debug::log(CRIT, "XDG_RUNTIME_DIR not set!");
        g_pHyprpicker->finish(1);
    }

    name = std::string(XDGRUNTIMEDIR) + "/.hyprpicker_XXXXXX";

    const auto FD = mkstemp((char*)name.c_str());
    if (FD < 0) {
        Debug::log(CRIT, "createPoolFile: fd < 0");
        g_pHyprpicker->finish(1);
    }

    if (!setCloexec(FD)) {
        close(FD);
        Debug::log(CRIT, "createPoolFile: !setCloexec");
        g_pHyprpicker->finish(1);
    }

    if (ftruncate(FD, size) < 0) {
        close(FD);
        Debug::log(CRIT, "createPoolFile: ftruncate < 0");
        g_pHyprpicker->finish(1);
    }

    return FD;
}

void CHyprpicker::createBuffer(SPoolBuffer* pBuffer, int32_t w, int32_t h, uint32_t format) {
    const uint STRIDE = w * 4;
    const size_t SIZE = STRIDE * h;

    std::string name;
    const auto FD = createPoolFile(SIZE, name);

    if (FD == -1) {
        Debug::log(CRIT, "Unable to create pool file!");
        g_pHyprpicker->finish(1);
    }

    const auto DATA = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, FD, 0);
    const auto POOL = wl_shm_create_pool(g_pHyprpicker->m_pWLSHM, FD, SIZE);
    pBuffer->buffer = wl_shm_pool_create_buffer(POOL, 0, w, h, STRIDE, format);

    wl_buffer_add_listener(pBuffer->buffer, &Events::bufferListener, pBuffer);

    wl_shm_pool_destroy(POOL);

    close(FD);

    pBuffer->format = format;
    pBuffer->size = SIZE;
    pBuffer->data = DATA;
    pBuffer->pixelSize = Vector2D(w, h);
    pBuffer->name = name;
}

void CHyprpicker::destroyBuffer(SPoolBuffer* pBuffer) {
    wl_buffer_destroy(pBuffer->buffer);
    cairo_destroy(pBuffer->cairo);
    cairo_surface_destroy(pBuffer->surface);
    munmap(pBuffer->data, pBuffer->size);

    pBuffer->buffer = nullptr;
    pBuffer->cairo = nullptr;
    pBuffer->surface = nullptr;

    unlink(pBuffer->name.c_str());
}

void CHyprpicker::createSeat(wl_seat* pSeat) {
    wl_seat_add_listener(pSeat, &Events::seatListener, pSeat);
}

void CHyprpicker::convertBuffer(SPoolBuffer* pBuffer) {
    switch (pBuffer->format) {
        case WL_SHM_FORMAT_ARGB8888:
        case WL_SHM_FORMAT_XRGB8888:
            break;
        case WL_SHM_FORMAT_ABGR8888:
        case WL_SHM_FORMAT_XBGR8888: {
            uint8_t* data = (uint8_t*)pBuffer->data;

            for (int y = 0; y < pBuffer->pixelSize.y; ++y) {
                for (int x = 0; x < pBuffer->pixelSize.x; ++x) {
                    struct pixel {
                        // little-endian ARGB
                        unsigned char blue;
                        unsigned char green;
                        unsigned char red;
                        unsigned char alpha;
                    }* px = (struct pixel*)(data + y * (int)pBuffer->pixelSize.x * 4 + x * 4);

                    std::swap(px->red, px->blue);
                }
            }
        }
        break;
        default: {
            Debug::log(CRIT, "Unsupported format %i", pBuffer->format);
        }
        g_pHyprpicker->finish(1);
    }
}

void CHyprpicker::renderSurface(CLayerSurface* pSurface, bool forceInactive) {
    const auto PBUFFER = getBufferForLS(pSurface);

    if (!PBUFFER || !pSurface->screenBuffer.buffer)
        return;

    if (!pSurface->screenBuffer.surface) {
        convertBuffer(&pSurface->screenBuffer);
        pSurface->screenBuffer.surface = cairo_image_surface_create_for_data((unsigned char*)pSurface->screenBuffer.data, CAIRO_FORMAT_ARGB32, pSurface->screenBuffer.pixelSize.x, pSurface->screenBuffer.pixelSize.y, pSurface->screenBuffer.pixelSize.x * 4);
    }

    PBUFFER->surface = cairo_image_surface_create_for_data((unsigned char*)PBUFFER->data, CAIRO_FORMAT_ARGB32, pSurface->m_pMonitor->size.x * pSurface->m_pMonitor->scale, pSurface->m_pMonitor->size.y * pSurface->m_pMonitor->scale, PBUFFER->pixelSize.x * 4);
    PBUFFER->cairo = cairo_create(PBUFFER->surface);

    const auto PCAIRO = PBUFFER->cairo;

    cairo_save(PCAIRO);

    cairo_set_source_rgba(PCAIRO, 0, 0, 0, 0);
    cairo_rectangle(PCAIRO, 0, 0, pSurface->m_pMonitor->size.x * pSurface->m_pMonitor->scale, pSurface->m_pMonitor->size.y * pSurface->m_pMonitor->scale);
    cairo_fill(PCAIRO);

    if (pSurface == g_pHyprpicker->m_pLastSurface && !forceInactive) {
        const auto SCALEBUFS = Vector2D{pSurface->screenBuffer.pixelSize.x / PBUFFER->pixelSize.x, pSurface->screenBuffer.pixelSize.y / PBUFFER->pixelSize.y};
        const auto SCALECURSOR = Vector2D{
            g_pHyprpicker->m_pLastSurface->screenBuffer.pixelSize.x / (g_pHyprpicker->m_pLastSurface->buffers[0].pixelSize.x / g_pHyprpicker->m_pLastSurface->m_pMonitor->scale),
            g_pHyprpicker->m_pLastSurface->screenBuffer.pixelSize.y / (g_pHyprpicker->m_pLastSurface->buffers[0].pixelSize.y / g_pHyprpicker->m_pLastSurface->m_pMonitor->scale)};
        const auto CLICKPOS = Vector2D{g_pHyprpicker->m_vLastCoords.floor().x * SCALECURSOR.x, g_pHyprpicker->m_vLastCoords.floor().y * SCALECURSOR.y};

        const auto PATTERNPRE = cairo_pattern_create_for_surface(pSurface->screenBuffer.surface);
        cairo_pattern_set_filter(PATTERNPRE, CAIRO_FILTER_BILINEAR);
        cairo_matrix_t matrixPre;
        cairo_matrix_init_identity(&matrixPre);
        cairo_matrix_scale(&matrixPre, SCALEBUFS.x, SCALEBUFS.y);
        cairo_pattern_set_matrix(PATTERNPRE, &matrixPre);
        cairo_set_source(PCAIRO, PATTERNPRE);
        cairo_paint(PCAIRO);

        cairo_surface_flush(PBUFFER->surface);

        cairo_pattern_destroy(PATTERNPRE);

        // we draw the preview like this
        //
        //     200px        ZOOM: 10x
        // | --------- |
        // |           |
        // |     x     | 200px
        // |           |
        // | --------- |
        //

    
        cairo_restore(PCAIRO);
        cairo_save(PCAIRO);

        const auto PIXCOLOR = getColorFromPixel(pSurface, CLICKPOS);
        cairo_set_source_rgba(PCAIRO, PIXCOLOR.r / 255.f, PIXCOLOR.g / 255.f, PIXCOLOR.b / 255.f, PIXCOLOR.a / 255.f);

        cairo_scale(PCAIRO, 1, 1);

        cairo_arc(PCAIRO, m_vLastCoords.x * pSurface->m_pMonitor->scale, m_vLastCoords.y * pSurface->m_pMonitor->scale, 105 / SCALEBUFS.x, 0, 2 * M_PI);
        cairo_clip(PCAIRO);

        cairo_fill(PCAIRO);
        cairo_paint(PCAIRO);

        cairo_surface_flush(PBUFFER->surface);

        cairo_restore(PCAIRO);
        cairo_save(PCAIRO);

        const auto PATTERN = cairo_pattern_create_for_surface(pSurface->screenBuffer.surface);
        cairo_pattern_set_filter(PATTERN, CAIRO_FILTER_NEAREST);
        cairo_matrix_t matrix;
        cairo_matrix_init_identity(&matrix);
        cairo_matrix_translate(&matrix, CLICKPOS.x + 0.5f, CLICKPOS.y + 0.5f);
        cairo_matrix_scale(&matrix, 0.1f, 0.1f);
        cairo_matrix_translate(&matrix, -CLICKPOS.x / SCALEBUFS.x - 0.5f, -CLICKPOS.y / SCALEBUFS.y - 0.5f);
        cairo_pattern_set_matrix(PATTERN, &matrix);
        cairo_set_source(PCAIRO, PATTERN);
        cairo_arc(PCAIRO, m_vLastCoords.x * pSurface->m_pMonitor->scale, m_vLastCoords.y * pSurface->m_pMonitor->scale, 100 / SCALEBUFS.x, 0, 2 * M_PI);
        cairo_clip(PCAIRO);
        cairo_paint(PCAIRO);

        cairo_surface_flush(PBUFFER->surface);

        cairo_restore(PCAIRO);

        cairo_pattern_destroy(PATTERN);
    } else {
        cairo_set_operator(PCAIRO, CAIRO_OPERATOR_SOURCE);
        cairo_set_source_rgba(PCAIRO, 0, 0, 0, 0);
        cairo_rectangle(PCAIRO, 0, 0, pSurface->m_pMonitor->size.x * pSurface->m_pMonitor->scale, pSurface->m_pMonitor->size.y * pSurface->m_pMonitor->scale);
        cairo_fill(PCAIRO);
    }

    sendFrame(pSurface);
    cairo_destroy(PCAIRO);
    cairo_surface_destroy(PBUFFER->surface);

    PBUFFER->cairo = nullptr;
    PBUFFER->surface = nullptr;

    pSurface->rendered = true;
}

void CHyprpicker::sendFrame(CLayerSurface* pSurface) {
    pSurface->frame_callback = wl_surface_frame(pSurface->pSurface);
    wl_callback_add_listener(pSurface->frame_callback, &Events::frameListener, pSurface);

    wl_surface_attach(pSurface->pSurface, pSurface->lastBuffer == 0 ? pSurface->buffers[0].buffer : pSurface->buffers[1].buffer, 0, 0);
    wl_surface_set_buffer_scale(pSurface->pSurface, pSurface->m_pMonitor->scale);
    wl_surface_damage_buffer(pSurface->pSurface, 0, 0, INT32_MAX, INT32_MAX);
    wl_surface_commit(pSurface->pSurface);

    pSurface->dirty = false;
}

CColor CHyprpicker::getColorFromPixel(CLayerSurface* pLS, Vector2D pix) {
    struct pixel {
        unsigned char blue;
        unsigned char green;
        unsigned char red;
        unsigned char alpha;
    }* px = (struct pixel*)(pLS->screenBuffer.data + (int)pix.y * (int)pLS->screenBuffer.pixelSize.x * 4 + (int)pix.x * 4);

    return CColor{(uint8_t)px->red, (uint8_t)px->green, (uint8_t)px->blue, (uint8_t)px->alpha};
}