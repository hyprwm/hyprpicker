#include "hyprpicker.hpp"
#include <csignal>

void sigHandler(int sig) {
    g_pHyprpicker->m_vLayerSurfaces.clear();
    exit(0);
}

void CHyprpicker::init() {
    m_pXKBContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!m_pXKBContext)
        Debug::log(ERR, "Failed to create xkb context");

    m_pWLDisplay = wl_display_connect(nullptr);

    if (!m_pWLDisplay) {
        Debug::log(CRIT, "No wayland compositor running!");
        exit(1);
        return;
    }

    signal(SIGTERM, sigHandler);

    m_pRegistry = makeShared<CCWlRegistry>((wl_proxy*)wl_display_get_registry(m_pWLDisplay));
    m_pRegistry->setGlobal([this](CCWlRegistry* r, uint32_t name, const char* interface, uint32_t version) {
        if (strcmp(interface, wl_compositor_interface.name) == 0) {
            m_pCompositor = makeShared<CCWlCompositor>((wl_proxy*)wl_registry_bind((wl_registry*)m_pRegistry->resource(), name, &wl_compositor_interface, 4));
        } else if (strcmp(interface, wl_shm_interface.name) == 0) {
            m_pSHM = makeShared<CCWlShm>((wl_proxy*)wl_registry_bind((wl_registry*)m_pRegistry->resource(), name, &wl_shm_interface, 1));
        } else if (strcmp(interface, wl_output_interface.name) == 0) {
            m_mtTickMutex.lock();

            const auto PMONITOR = g_pHyprpicker->m_vMonitors
                                      .emplace_back(std::make_unique<SMonitor>(
                                          makeShared<CCWlOutput>((wl_proxy*)wl_registry_bind((wl_registry*)m_pRegistry->resource(), name, &wl_output_interface, 4))))
                                      .get();
            PMONITOR->wayland_name = name;

            m_mtTickMutex.unlock();
        } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
            m_pLayerShell = makeShared<CCZwlrLayerShellV1>((wl_proxy*)wl_registry_bind((wl_registry*)m_pRegistry->resource(), name, &zwlr_layer_shell_v1_interface, 1));
        } else if (strcmp(interface, wl_seat_interface.name) == 0) {
            m_pSeat = makeShared<CCWlSeat>((wl_proxy*)wl_registry_bind((wl_registry*)m_pRegistry->resource(), name, &wl_seat_interface, 1));

            m_pSeat->setCapabilities([this](CCWlSeat* seat, uint32_t caps) {
                if (caps & WL_SEAT_CAPABILITY_POINTER) {
                    if (!m_pPointer) {
                        m_pPointer = makeShared<CCWlPointer>(m_pSeat->sendGetPointer());
                        initMouse();
                        if (m_pCursorShapeMgr)
                            m_pCursorShapeDevice = makeShared<CCWpCursorShapeDeviceV1>(m_pCursorShapeMgr->sendGetPointer(m_pPointer->resource()));
                    }
                } else {
                    Debug::log(CRIT, "Hyprpicker cannot work without a pointer!");
                    g_pHyprpicker->finish(1);
                }

                if (caps & WL_SEAT_CAPABILITY_KEYBOARD) {
                    if (!m_pKeyboard) {
                        m_pKeyboard = makeShared<CCWlKeyboard>(m_pSeat->sendGetKeyboard());
                        initKeyboard();
                    }
                } else
                    m_pKeyboard.reset();
            });

        } else if (strcmp(interface, zwlr_screencopy_manager_v1_interface.name) == 0) {
            m_pScreencopyMgr =
                makeShared<CCZwlrScreencopyManagerV1>((wl_proxy*)wl_registry_bind((wl_registry*)m_pRegistry->resource(), name, &zwlr_screencopy_manager_v1_interface, 1));
        } else if (strcmp(interface, wp_cursor_shape_manager_v1_interface.name) == 0) {
            m_pCursorShapeMgr =
                makeShared<CCWpCursorShapeManagerV1>((wl_proxy*)wl_registry_bind((wl_registry*)m_pRegistry->resource(), name, &wp_cursor_shape_manager_v1_interface, 1));
        } else if (strcmp(interface, wp_fractional_scale_manager_v1_interface.name) == 0) {
            m_pFractionalMgr =
                makeShared<CCWpFractionalScaleManagerV1>((wl_proxy*)wl_registry_bind((wl_registry*)m_pRegistry->resource(), name, &wp_fractional_scale_manager_v1_interface, 1));
        } else if (strcmp(interface, wp_viewporter_interface.name) == 0) {
            m_pViewporter = makeShared<CCWpViewporter>((wl_proxy*)wl_registry_bind((wl_registry*)m_pRegistry->resource(), name, &wp_viewporter_interface, 1));
        }
    });

    wl_display_roundtrip(m_pWLDisplay);

    if (!m_pCursorShapeMgr)
        Debug::log(ERR, "cursor_shape_v1 not supported, cursor won't be affected");

    if (!m_pScreencopyMgr) {
        Debug::log(CRIT, "zwlr_screencopy_v1 not supported, can't proceed");
        exit(1);
    }

    if (!m_pFractionalMgr) {
        Debug::log(WARN, "wp_fractional_scale_v1 not supported, fractional scaling won't work");
        m_bNoFractional = true;
    }
    if (!m_pViewporter) {
        Debug::log(WARN, "wp_viewporter not supported, fractional scaling won't work");
        m_bNoFractional = true;
    }

    for (auto& m : m_vMonitors) {
        m_vLayerSurfaces.emplace_back(std::make_unique<CLayerSurface>(m.get()));

        m_pLastSurface = m_vLayerSurfaces.back().get();

        m->pSCFrame = makeShared<CCZwlrScreencopyFrameV1>(m_pScreencopyMgr->sendCaptureOutput(false, m->output->resource()));
        m->pLS      = m_vLayerSurfaces.back().get();
        m->initSCFrame();
    }

    wl_display_roundtrip(m_pWLDisplay);

    while (m_bRunning && wl_display_dispatch(m_pWLDisplay) != -1) {
        //renderSurface(m_pLastSurface);
    }

    if (m_pWLDisplay) {
        wl_display_disconnect(m_pWLDisplay);
        m_pWLDisplay = nullptr;
    }
}

void CHyprpicker::finish(int code) {
    m_vLayerSurfaces.clear();

    if (m_pWLDisplay) {
        m_vLayerSurfaces.clear();
        m_vMonitors.clear();
        m_pCompositor.reset();
        m_pRegistry.reset();
        m_pSHM.reset();
        m_pLayerShell.reset();
        m_pScreencopyMgr.reset();
        m_pCursorShapeMgr.reset();
        m_pCursorShapeDevice.reset();
        m_pSeat.reset();
        m_pKeyboard.reset();
        m_pPointer.reset();
        m_pViewporter.reset();
        m_pFractionalMgr.reset();

        wl_display_disconnect(m_pWLDisplay);
        m_pWLDisplay = nullptr;
    }

    exit(code);
}

void CHyprpicker::recheckACK() {
    for (auto& ls : m_vLayerSurfaces) {
        if ((ls->wantsACK || ls->wantsReload) && ls->screenBuffer) {
            if (ls->wantsACK)
                ls->pLayerSurface->sendAckConfigure(ls->ACKSerial);
            ls->wantsACK    = false;
            ls->wantsReload = false;

            const auto MONITORSIZE =
                (ls->screenBuffer && !g_pHyprpicker->m_bNoFractional ? ls->m_pMonitor->size * ls->fractionalScale : ls->m_pMonitor->size * ls->m_pMonitor->scale).round();

            if (!ls->buffers[0] || ls->buffers[0]->pixelSize != MONITORSIZE) {
                Debug::log(TRACE, "making new buffers: size changed to %.0fx%.0f", MONITORSIZE.x, MONITORSIZE.y);
                ls->buffers[0] = makeShared<SPoolBuffer>(MONITORSIZE, WL_SHM_FORMAT_ARGB8888, MONITORSIZE.x * 4);
                ls->buffers[1] = makeShared<SPoolBuffer>(MONITORSIZE, WL_SHM_FORMAT_ARGB8888, MONITORSIZE.x * 4);
            }
        }
    }

    markDirty();
}

void CHyprpicker::markDirty() {
    for (auto& ls : m_vLayerSurfaces) {
        if (ls->frameCallback)
            continue;

        ls->markDirty();
    }
}

SP<SPoolBuffer> CHyprpicker::getBufferForLS(CLayerSurface* pLS) {
    SP<SPoolBuffer> returns = nullptr;

    for (auto i = 0; i < 2; ++i) {
        if (!pLS->buffers[i] || pLS->buffers[i]->busy)
            continue;

        returns = pLS->buffers[i];
    }

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

void CHyprpicker::convertBuffer(SP<SPoolBuffer> pBuffer) {
    switch (pBuffer->format) {
        case WL_SHM_FORMAT_ARGB8888:
        case WL_SHM_FORMAT_XRGB8888: break;
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
                    }* px = (struct pixel*)(data + (y * (int)pBuffer->pixelSize.x * 4) + (x * 4));

                    std::swap(px->red, px->blue);
                }
            }
        } break;
        case WL_SHM_FORMAT_XRGB2101010:
        case WL_SHM_FORMAT_XBGR2101010: {
            uint8_t*   data = (uint8_t*)pBuffer->data;

            const bool FLIP = pBuffer->format == WL_SHM_FORMAT_XBGR2101010;

            for (int y = 0; y < pBuffer->pixelSize.y; ++y) {
                for (int x = 0; x < pBuffer->pixelSize.x; ++x) {
                    uint32_t* px = (uint32_t*)(data + (y * (int)pBuffer->pixelSize.x * 4) + (x * 4));

                    // conv to 8 bit
                    uint8_t R = (uint8_t)std::round((255.0 * (((*px) & 0b00000000000000000000001111111111) >> 0) / 1023.0));
                    uint8_t G = (uint8_t)std::round((255.0 * (((*px) & 0b00000000000011111111110000000000) >> 10) / 1023.0));
                    uint8_t B = (uint8_t)std::round((255.0 * (((*px) & 0b00111111111100000000000000000000) >> 20) / 1023.0));
                    uint8_t A = (uint8_t)std::round((255.0 * (((*px) & 0b11000000000000000000000000000000) >> 30) / 3.0));

                    // write 8-bit values
                    *px = ((FLIP ? B : R) << 0) + (G << 8) + ((FLIP ? R : B) << 16) + (A << 24);
                }
            }
        } break;
        default: {
            Debug::log(CRIT, "Unsupported format %i", pBuffer->format);
        }
            g_pHyprpicker->finish(1);
    }
}

// Mallocs a new buffer, which needs to be free'd!
void* CHyprpicker::convert24To32Buffer(SP<SPoolBuffer> pBuffer) {
    uint8_t* newBuffer       = (uint8_t*)malloc((size_t)pBuffer->pixelSize.x * pBuffer->pixelSize.y * 4);
    int      newBufferStride = pBuffer->pixelSize.x * 4;
    uint8_t* oldBuffer       = (uint8_t*)pBuffer->data;

    switch (pBuffer->format) {
        case WL_SHM_FORMAT_BGR888: {
            for (int y = 0; y < pBuffer->pixelSize.y; ++y) {
                for (int x = 0; x < pBuffer->pixelSize.x; ++x) {
                    struct pixel3 {
                        // little-endian RGB
                        unsigned char blue;
                        unsigned char green;
                        unsigned char red;
                    }* srcPx = (struct pixel3*)(oldBuffer + (y * pBuffer->stride) + (x * 3));
                    struct pixel4 {
                        // little-endian ARGB
                        unsigned char blue;
                        unsigned char green;
                        unsigned char red;
                        unsigned char alpha;
                    }* dstPx = (struct pixel4*)(newBuffer + (y * newBufferStride) + (x * 4));
                    *dstPx   = {.blue = srcPx->red, .green = srcPx->green, .red = srcPx->blue, .alpha = 0xFF};
                }
            }
        } break;
        case WL_SHM_FORMAT_RGB888: {
            for (int y = 0; y < pBuffer->pixelSize.y; ++y) {
                for (int x = 0; x < pBuffer->pixelSize.x; ++x) {
                    struct pixel3 {
                        // big-endian RGB
                        unsigned char red;
                        unsigned char green;
                        unsigned char blue;
                    }* srcPx = (struct pixel3*)(oldBuffer + (y * pBuffer->stride) + (x * 3));
                    struct pixel4 {
                        // big-endian ARGB
                        unsigned char alpha;
                        unsigned char red;
                        unsigned char green;
                        unsigned char blue;
                    }* dstPx = (struct pixel4*)(newBuffer + (y * newBufferStride) + (x * 4));
                    *dstPx   = {.alpha = 0xFF, .red = srcPx->red, .green = srcPx->green, .blue = srcPx->blue};
                }
            }
        } break;
        default: {
            Debug::log(CRIT, "Unsupported format for 24bit buffer %i", pBuffer->format);
        }
            g_pHyprpicker->finish(1);
    }
    return newBuffer;
}

void CHyprpicker::renderSurface(CLayerSurface* pSurface, bool forceInactive) {
    const auto PBUFFER = getBufferForLS(pSurface);

    if (!PBUFFER || !pSurface->screenBuffer) {
        // Debug::log(ERR, PBUFFER ? "renderSurface: pSurface->screenBuffer null" : "renderSurface: PBUFFER null");
        return;
    }

    PBUFFER->surface =
        cairo_image_surface_create_for_data((unsigned char*)PBUFFER->data, CAIRO_FORMAT_ARGB32, PBUFFER->pixelSize.x, PBUFFER->pixelSize.y, PBUFFER->pixelSize.x * 4);

    PBUFFER->cairo = cairo_create(PBUFFER->surface);

    const auto PCAIRO = PBUFFER->cairo;

    cairo_save(PCAIRO);

    cairo_set_source_rgba(PCAIRO, 0, 0, 0, 0);

    cairo_rectangle(PCAIRO, 0, 0, PBUFFER->pixelSize.x, PBUFFER->pixelSize.y);
    cairo_fill(PCAIRO);

    if (pSurface == m_pLastSurface && !forceInactive) {
        const auto SCALEBUFS      = pSurface->screenBuffer->pixelSize / PBUFFER->pixelSize;
        const auto MOUSECOORDSABS = m_vLastCoords.floor() / pSurface->m_pMonitor->size;
        const auto CLICKPOS       = MOUSECOORDSABS * PBUFFER->pixelSize;

        Debug::log(TRACE, "renderSurface: scalebufs %.2fx%.2f", SCALEBUFS.x, SCALEBUFS.y);

        const auto PATTERNPRE = cairo_pattern_create_for_surface(pSurface->screenBuffer->surface);
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
        // (hex code here)

        cairo_restore(PCAIRO);
        if (!m_bNoZoom) {
            cairo_save(PCAIRO);

            const auto CLICKPOSBUF = CLICKPOS / PBUFFER->pixelSize * pSurface->screenBuffer->pixelSize;

            const auto PIXCOLOR = getColorFromPixel(pSurface, CLICKPOSBUF);
            cairo_set_source_rgba(PCAIRO, PIXCOLOR.r / 255.f, PIXCOLOR.g / 255.f, PIXCOLOR.b / 255.f, PIXCOLOR.a / 255.f);

            cairo_scale(PCAIRO, 1, 1);

            cairo_arc(PCAIRO, CLICKPOS.x, CLICKPOS.y, 105 / SCALEBUFS.x, 0, 2 * M_PI);
            cairo_clip(PCAIRO);

            cairo_fill(PCAIRO);
            cairo_paint(PCAIRO);

            cairo_surface_flush(PBUFFER->surface);

            cairo_restore(PCAIRO);
            cairo_save(PCAIRO);

            const auto PATTERN = cairo_pattern_create_for_surface(pSurface->screenBuffer->surface);
            cairo_pattern_set_filter(PATTERN, CAIRO_FILTER_NEAREST);
            cairo_matrix_t matrix;
            cairo_matrix_init_identity(&matrix);
            cairo_matrix_translate(&matrix, CLICKPOSBUF.x + 0.5f, CLICKPOSBUF.y + 0.5f);
            cairo_matrix_scale(&matrix, 0.1f, 0.1f);
            cairo_matrix_translate(&matrix, (-CLICKPOSBUF.x / SCALEBUFS.x) - 0.5f, (-CLICKPOSBUF.y / SCALEBUFS.y) - 0.5f);
            cairo_pattern_set_matrix(PATTERN, &matrix);
            cairo_set_source(PCAIRO, PATTERN);
            cairo_arc(PCAIRO, CLICKPOS.x, CLICKPOS.y, 100 / SCALEBUFS.x, 0, 2 * M_PI);
            cairo_clip(PCAIRO);
            cairo_paint(PCAIRO);

            if (!m_bDisableHexPreview) {
                const auto  currentColor = getColorFromPixel(pSurface, CLICKPOS);
                std::string hexBuffer;
                if (m_bUseLowerCase)
                    hexBuffer = std::format("#{:02x}{:02x}{:02x}", currentColor.r, currentColor.g, currentColor.b);
                else
                    hexBuffer = std::format("#{:02X}{:02X}{:02X}", currentColor.r, currentColor.g, currentColor.b);

                cairo_set_source_rgba(PCAIRO, 0.0, 0.0, 0.0, 0.5);

                double x, y, width = 85, height = 28, radius = 6;

                if (CLICKPOS.y > (PBUFFER->pixelSize.y - 50) && CLICKPOS.x > (PBUFFER->pixelSize.x - 100)) {
                    x = CLICKPOS.x - 80;
                    y = CLICKPOS.y - 40;
                } else if (CLICKPOS.y > (PBUFFER->pixelSize.y - 50)) {
                    x = CLICKPOS.x;
                    y = CLICKPOS.y - 40;
                } else if (CLICKPOS.x > (PBUFFER->pixelSize.x - 100)) {
                    x = CLICKPOS.x - 80;
                    y = CLICKPOS.y + 20;
                } else {
                    x = CLICKPOS.x;
                    y = CLICKPOS.y + 20;
                }

                cairo_move_to(PCAIRO, x + radius, y);
                cairo_arc(PCAIRO, x + width - radius, y + radius, radius, -M_PI_2, 0);
                cairo_arc(PCAIRO, x + width - radius, y + height - radius, radius, 0, M_PI_2);
                cairo_arc(PCAIRO, x + radius, y + height - radius, radius, M_PI_2, M_PI);
                cairo_arc(PCAIRO, x + radius, y + radius, radius, M_PI, -M_PI_2);

                cairo_close_path(PCAIRO);
                cairo_fill(PCAIRO);

                cairo_set_source_rgba(PCAIRO, 1.0, 1.0, 1.0, 1.0);
                cairo_select_font_face(PCAIRO, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
                cairo_set_font_size(PCAIRO, 18);

                double padding = 5.0;
                double textX   = x + padding;

                if (CLICKPOS.y > (PBUFFER->pixelSize.y - 50) && CLICKPOS.x > (PBUFFER->pixelSize.x - 100))
                    cairo_move_to(PCAIRO, textX, CLICKPOS.y - 20);
                else if (CLICKPOS.y > (PBUFFER->pixelSize.y - 50))
                    cairo_move_to(PCAIRO, textX, CLICKPOS.y - 20);
                else if (CLICKPOS.x > (PBUFFER->pixelSize.x - 100))
                    cairo_move_to(PCAIRO, textX, CLICKPOS.y + 40);
                else
                    cairo_move_to(PCAIRO, textX, CLICKPOS.y + 40);

                cairo_show_text(PCAIRO, hexBuffer.c_str());

                cairo_surface_flush(PBUFFER->surface);
            }
            cairo_restore(PCAIRO);
            cairo_pattern_destroy(PATTERN);
        }
    } else if (!m_bRenderInactive) {
        cairo_set_operator(PCAIRO, CAIRO_OPERATOR_SOURCE);
        cairo_set_source_rgba(PCAIRO, 0, 0, 0, 0);
        cairo_rectangle(PCAIRO, 0, 0, PBUFFER->pixelSize.x, PBUFFER->pixelSize.y);
        cairo_fill(PCAIRO);
    } else {
        const auto SCALEBUFS  = pSurface->screenBuffer->pixelSize / PBUFFER->pixelSize;
        const auto PATTERNPRE = cairo_pattern_create_for_surface(pSurface->screenBuffer->surface);
        cairo_pattern_set_filter(PATTERNPRE, CAIRO_FILTER_BILINEAR);
        cairo_matrix_t matrixPre;
        cairo_matrix_init_identity(&matrixPre);
        cairo_matrix_scale(&matrixPre, SCALEBUFS.x, SCALEBUFS.y);
        cairo_pattern_set_matrix(PATTERNPRE, &matrixPre);
        cairo_set_source(PCAIRO, PATTERNPRE);
        cairo_paint(PCAIRO);

        cairo_surface_flush(PBUFFER->surface);
        cairo_pattern_destroy(PATTERNPRE);
    }

    pSurface->sendFrame();
    cairo_destroy(PCAIRO);
    cairo_surface_destroy(PBUFFER->surface);

    PBUFFER->busy    = true;
    PBUFFER->cairo   = nullptr;
    PBUFFER->surface = nullptr;

    pSurface->rendered = true;
}

CColor CHyprpicker::getColorFromPixel(CLayerSurface* pLS, Vector2D pix) {
    pix = pix.floor();

    if (pix.x >= pLS->screenBuffer->pixelSize.x || pix.y >= pLS->screenBuffer->pixelSize.y || pix.x < 0 || pix.y < 0)
        return CColor{.r = 0, .g = 0, .b = 0, .a = 0};

    void* dataSrc = pLS->screenBuffer->paddedData ? pLS->screenBuffer->paddedData : pLS->screenBuffer->data;
    struct pixel {
        unsigned char blue;
        unsigned char green;
        unsigned char red;
        unsigned char alpha;
        // Implicit widening to the ptrdiff_t happens with the pix.y and pix.x * 4. Maybe cast them to the ptrdiff_t and not to int?
    }* px = (struct pixel*)((char*)dataSrc + ((int)pix.y * (int)pLS->screenBuffer->pixelSize.x * 4) + ((int)pix.x * 4));

    return CColor{.r = px->red, .g = px->green, .b = px->blue, .a = px->alpha};
}

void CHyprpicker::initKeyboard() {
    m_pKeyboard->setKeymap([this](CCWlKeyboard* r, wl_keyboard_keymap_format format, int32_t fd, uint32_t size) {
        if (!m_pXKBContext)
            return;

        if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
            Debug::log(ERR, "Could not recognise keymap format");
            return;
        }

        const char* buf = (const char*)mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);
        if (buf == MAP_FAILED) {
            Debug::log(ERR, "Failed to mmap xkb keymap: %d", errno);
            return;
        }

        m_pXKBKeymap = xkb_keymap_new_from_buffer(m_pXKBContext, buf, size - 1, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);

        munmap((void*)buf, size);
        close(fd);

        if (!m_pXKBKeymap) {
            Debug::log(ERR, "Failed to compile xkb keymap");
            return;
        }

        m_pXKBState = xkb_state_new(m_pXKBKeymap);
        if (!m_pXKBState) {
            Debug::log(ERR, "Failed to create xkb state");
            return;
        }
    });

    m_pKeyboard->setKey([this](CCWlKeyboard* r, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
        if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
            return;

        if (m_pXKBState) {
            if (xkb_state_key_get_one_sym(m_pXKBState, key + 8) == XKB_KEY_Escape)
                finish();
        } else if (key == 1) // Assume keycode 1 is escape
            finish();
    });
}

void CHyprpicker::initMouse() {
    m_pPointer->setEnter([this](CCWlPointer* r, uint32_t serial, wl_proxy* surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
        auto x = wl_fixed_to_double(surface_x);
        auto y = wl_fixed_to_double(surface_y);

        m_vLastCoords = {x, y};

        markDirty();

        for (auto& ls : m_vLayerSurfaces) {
            if (ls->pSurface->resource() == surface) {
                m_pLastSurface = ls.get();
                break;
            }
        }

        m_pCursorShapeDevice->sendSetShape(serial, WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_CROSSHAIR);
    });
    m_pPointer->setLeave([this](CCWlPointer* r, uint32_t timeMs, wl_proxy* surf) {
        for (auto& ls : m_vLayerSurfaces) {
            if (ls->pSurface->resource() == surf) {
                renderSurface(ls.get(), true);
            }
        }
    });
    m_pPointer->setMotion([this](CCWlPointer* r, uint32_t timeMs, wl_fixed_t surface_x, wl_fixed_t surface_y) {
        auto x = wl_fixed_to_double(surface_x);
        auto y = wl_fixed_to_double(surface_y);

        m_vLastCoords = {x, y};

        markDirty();
    });
    m_pPointer->setButton([this](CCWlPointer* r, uint32_t serial, uint32_t time, uint32_t button, uint32_t button_state) {
        auto fmax3 = [](float a, float b, float c) -> float { return (a > b && a > c) ? a : (b > c) ? b : c; };
        auto fmin3 = [](float a, float b, float c) -> float { return (a < b && a < c) ? a : (b < c) ? b : c; };

        // relative brightness of a color
        // https://www.w3.org/TR/2008/REC-WCAG20-20081211/#relativeluminancedef
        const auto FLUMI = [](const float& c) -> float { return c <= 0.03928 ? c / 12.92 : powf((c + 0.055) / 1.055, 2.4); };

        // get the px and print it
        const auto MOUSECOORDSABS = m_vLastCoords.floor() / m_pLastSurface->m_pMonitor->size;
        const auto CLICKPOS       = MOUSECOORDSABS * m_pLastSurface->screenBuffer->pixelSize;

        const auto COL = getColorFromPixel(m_pLastSurface, CLICKPOS);

        // threshold: (lumi_white + 0.05) / (x + 0.05) == (x + 0.05) / (lumi_black + 0.05)
        // https://www.w3.org/TR/2008/REC-WCAG20-20081211/#contrast-ratiodef
        const uint8_t FG = 0.2126 * FLUMI(COL.r / 255.0f) + 0.7152 * FLUMI(COL.g / 255.0f) + 0.0722 * FLUMI(COL.b / 255.0f) > 0.17913 ? 0 : 255;

        switch (m_bSelectedOutputMode) {
            case OUTPUT_CMYK: {
                // http://www.codeproject.com/KB/applications/xcmyk.aspx

                float r = 1 - (COL.r / 255.0f), g = 1 - (COL.g / 255.0f), b = 1 - (COL.b / 255.0f);
                float k = fmin3(r, g, b), K = (k == 1) ? 1 : 1 - k;
                float c = (r - k) / K, m = (g - k) / K, y = (b - k) / K;

                c = std::round(c * 100);
                m = std::round(m * 100);
                y = std::round(y * 100);
                k = std::round(k * 100);

                if (m_bFancyOutput)
                    Debug::log(NONE, "\033[38;2;%i;%i;%i;48;2;%i;%i;%im%g%% %g%% %g%% %g%%\033[0m", FG, FG, FG, COL.r, COL.g, COL.b, c, m, y, k);
                else
                    Debug::log(NONE, "%g%% %g%% %g%% %g%%", c, m, y, k);

                if (m_bAutoCopy)
                    Clipboard::copy("%g%% %g%% %g%% %g%%", c, m, y, k);
                finish();
                break;
            }
            case OUTPUT_HEX: {
                auto toHex = [this](int i) -> std::string {
                    const char* DS = m_bUseLowerCase ? "0123456789abcdef" : "0123456789ABCDEF";

                    std::string result = "";

                    result += DS[i / 16];
                    result += DS[i % 16];

                    return result;
                };
                // Use or delete?
                auto hexR = toHex(COL.r);
                auto hexG = toHex(COL.g);
                auto hexB = toHex(COL.b);

                if (m_bFancyOutput)
                    Debug::log(NONE, "\033[38;2;%i;%i;%i;48;2;%i;%i;%im#%s%s%s\033[0m", FG, FG, FG, COL.r, COL.g, COL.b, toHex(COL.r).c_str(), toHex(COL.g).c_str(),
                               toHex(COL.b).c_str());
                else
                    Debug::log(NONE, "#%s%s%s", toHex(COL.r).c_str(), toHex(COL.g).c_str(), toHex(COL.b).c_str());

                if (m_bAutoCopy)
                    Clipboard::copy("#%s%s%s", toHex(COL.r).c_str(), toHex(COL.g).c_str(), toHex(COL.b).c_str());
                finish();
                break;
            }
            case OUTPUT_RGB: {
                if (m_bFancyOutput)
                    Debug::log(NONE, "\033[38;2;%i;%i;%i;48;2;%i;%i;%im%i %i %i\033[0m", FG, FG, FG, COL.r, COL.g, COL.b, COL.r, COL.g, COL.b);
                else
                    Debug::log(NONE, "%i %i %i", COL.r, COL.g, COL.b);

                if (m_bAutoCopy)
                    Clipboard::copy("%i %i %i", COL.r, COL.g, COL.b);
                finish();
                break;
            }
            case OUTPUT_HSL:
            case OUTPUT_HSV: {
                // https://en.wikipedia.org/wiki/HSL_and_HSV#From_RGB

                auto floatEq = [](float a, float b) -> bool {
                    return std::nextafter(a, std::numeric_limits<double>::lowest()) <= b && std::nextafter(a, std::numeric_limits<double>::max()) >= b;
                };

                float h, s, l, v;
                float r = COL.r / 255.0f, g = COL.g / 255.0f, b = COL.b / 255.0f;
                float max = fmax3(r, g, b), min = fmin3(r, g, b);
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
                if (m_bSelectedOutputMode == OUTPUT_HSL) {
                    l      = (max + min) / 2;
                    s      = (floatEq(l, 0.0f) || floatEq(l, 1.0f)) ? 0 : (v - l) / std::min(l, 1 - l);
                    l_or_v = std::round(l * 100);
                } else {
                    v      = max;
                    s      = floatEq(v, 0.0f) ? 0 : c / v;
                    l_or_v = std::round(v * 100);
                }

                h = std::round(h);
                s = std::round(s * 100);

                if (m_bFancyOutput)
                    Debug::log(NONE, "\033[38;2;%i;%i;%i;48;2;%i;%i;%im%g %g%% %g%%\033[0m", FG, FG, FG, COL.r, COL.g, COL.b, h, s, l_or_v);
                else
                    Debug::log(NONE, "%g %g%% %g%%", h, s, l_or_v);

                if (m_bAutoCopy)
                    Clipboard::copy("%g %g%% %g%%", h, s, l_or_v);
                finish();
                break;
            }
        }

        finish();
    });
}
