#include "Monitor.hpp"
#include "LayerSurface.hpp"
#include "../hyprpicker.hpp"

SMonitor::SMonitor(SP<CCWlOutput> output_) : output(output_) {
    output->setGeometry([this](CCWlOutput* r, int32_t x, int32_t y, int32_t width_mm, int32_t height_mm, int32_t subpixel, const char* make, const char* model,
                               int32_t transform_) { //
        transform = (wl_output_transform)transform_;
    });
    output->setDone([this](CCWlOutput* r) { //
        ready = true;
    });
    output->setScale([this](CCWlOutput* r, int32_t scale_) { //
        scale = scale_;
    });
    output->setName([this](CCWlOutput* r, const char* name_) { //
        if (name_)
            name = name_;
    });
}

void SMonitor::initSCFrame() {
    pSCFrame->setBuffer([this](CCZwlrScreencopyFrameV1* r, uint32_t format, uint32_t width, uint32_t height, uint32_t stride) {
        pLS->screenBufferFormat = format;

        if (!pLS->screenBuffer)
            pLS->screenBuffer = makeShared<SPoolBuffer>(Vector2D{(double)width, (double)height}, format, stride);

        pSCFrame->sendCopy(pLS->screenBuffer->buffer->resource());
    });
    pSCFrame->setFlags([this](CCZwlrScreencopyFrameV1* r, uint32_t flags) {
        pLS->scflags = flags;

        g_pHyprpicker->recheckACK();
    });
    pSCFrame->setReady([this](CCZwlrScreencopyFrameV1* r, uint32_t tv_sec_hi, uint32_t tv_sec_lo, uint32_t tv_nsec) {
        Vector2D transformedSize = pLS->screenBuffer->pixelSize;

        if (pLS->m_pMonitor->transform % 2 == 1)
            std::swap(transformedSize.x, transformedSize.y);

        Debug::log(TRACE, "Frame ready: pixel %.0fx%.0f, xfmd: %.0fx%.0f", pLS->screenBuffer->pixelSize.x, pLS->screenBuffer->pixelSize.y, transformedSize.x, transformedSize.y);

        SP<SPoolBuffer> newBuf = makeShared<SPoolBuffer>(transformedSize, pLS->screenBufferFormat, transformedSize.x * 4);

        int             bytesPerPixel = pLS->screenBuffer->stride / (int)pLS->screenBuffer->pixelSize.x;
        void*           data          = pLS->screenBuffer->data;
        if (bytesPerPixel == 4)
            g_pHyprpicker->convertBuffer(pLS->screenBuffer);
        else if (bytesPerPixel == 3) {
            Debug::log(WARN, "24 bit formats are unsupported, hyprpicker may or may not work as intended!");
            data                          = g_pHyprpicker->convert24To32Buffer(pLS->screenBuffer);
            pLS->screenBuffer->paddedData = data;
        } else {
            Debug::log(CRIT, "Unsupported stride/bytes per pixel %i", bytesPerPixel);
            g_pHyprpicker->finish(1);
        }

        cairo_surface_t* oldSurface = cairo_image_surface_create_for_data((unsigned char*)data, CAIRO_FORMAT_ARGB32, pLS->screenBuffer->pixelSize.x, pLS->screenBuffer->pixelSize.y,
                                                                          pLS->screenBuffer->pixelSize.x * 4);

        cairo_surface_flush(oldSurface);

        newBuf->surface = cairo_image_surface_create_for_data((unsigned char*)newBuf->data, CAIRO_FORMAT_ARGB32, transformedSize.x, transformedSize.y, transformedSize.x * 4);

        const auto PCAIRO = cairo_create(newBuf->surface);

        auto       cairoTransformMtx = [&](cairo_matrix_t* mtx) -> void {
            const auto TR = pLS->m_pMonitor->transform % 4;

            if (TR == 0)
                return;

            cairo_matrix_rotate(mtx, -M_PI_2 * (double)TR);

            if (TR == 1)
                cairo_matrix_translate(mtx, -transformedSize.x, 0);
            else if (TR == 2)
                cairo_matrix_translate(mtx, -transformedSize.x, -transformedSize.y);
            else if (TR == 3)
                cairo_matrix_translate(mtx, 0, -transformedSize.y);

            // TODO: flipped
        };

        cairo_save(PCAIRO);

        cairo_set_source_rgba(PCAIRO, 0, 0, 0, 0);

        cairo_rectangle(PCAIRO, 0, 0, 0xFFFF, 0xFFFF);
        cairo_fill(PCAIRO);

        const auto PATTERNPRE = cairo_pattern_create_for_surface(oldSurface);
        cairo_pattern_set_filter(PATTERNPRE, CAIRO_FILTER_BILINEAR);
        cairo_matrix_t matrixPre;
        cairo_matrix_init_identity(&matrixPre);
        cairo_matrix_scale(&matrixPre, 1.0, 1.0);
        cairoTransformMtx(&matrixPre);
        cairo_pattern_set_matrix(PATTERNPRE, &matrixPre);
        cairo_set_source(PCAIRO, PATTERNPRE);
        cairo_paint(PCAIRO);

        cairo_surface_flush(newBuf->surface);

        cairo_pattern_destroy(PATTERNPRE);

        cairo_destroy(PCAIRO);

        cairo_surface_destroy(oldSurface);

        pLS->screenBuffer = newBuf;

        g_pHyprpicker->renderSurface(pLS);

        pSCFrame.reset();
    });
    pSCFrame->setFailed([](CCZwlrScreencopyFrameV1* r) {
        Debug::log(CRIT, "Failed to get a Screencopy!");
        g_pHyprpicker->finish(1);
    });
}
