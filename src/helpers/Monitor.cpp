#include "Monitor.hpp"
#include "LayerSurface.hpp"
#include "../hyprpicker.hpp"
#include <array>
#include <limits>
#include <optional>

SMonitor::SMonitor(SP<CCWlOutput> output_) : output(output_) {
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

namespace {
    struct SCaptureFormat {
        uint32_t format;
        uint32_t bytesPerPixel;
    };

    constexpr std::array<SCaptureFormat, 8> CAPTURE_FORMATS = {{{WL_SHM_FORMAT_XRGB8888, 4},
                                                                {WL_SHM_FORMAT_ARGB8888, 4},
                                                                {WL_SHM_FORMAT_XBGR8888, 4},
                                                                {WL_SHM_FORMAT_ABGR8888, 4},
                                                                {WL_SHM_FORMAT_XRGB2101010, 4},
                                                                {WL_SHM_FORMAT_XBGR2101010, 4},
                                                                {WL_SHM_FORMAT_BGR888, 3},
                                                                {WL_SHM_FORMAT_RGB888, 3}}};

    std::optional<SCaptureFormat>           selectCaptureFormat(const std::vector<uint32_t>& advertisedFormats) {
        for (const auto& candidate : CAPTURE_FORMATS) {
            if (std::find(advertisedFormats.begin(), advertisedFormats.end(), candidate.format) != advertisedFormats.end())
                return candidate;
        }

        return std::nullopt;
    }
}

void SMonitor::initCapture() {
    pCaptureSource = makeShared<CCExtImageCaptureSourceV1>(g_pHyprpicker->m_pImageCaptureSourceMgr->sendCreateSource(output->resource()));

    auto OPTIONS = static_cast<extImageCopyCaptureManagerV1Options>(0);
    if (g_pHyprpicker->m_bIncludeCursor)
        OPTIONS = EXT_IMAGE_COPY_CAPTURE_MANAGER_V1_OPTIONS_PAINT_CURSORS;
    pCaptureSession = makeShared<CCExtImageCopyCaptureSessionV1>(g_pHyprpicker->m_pImageCopyCaptureMgr->sendCreateSession(pCaptureSource->resource(), OPTIONS));

    pCaptureSession->setBufferSize([this](CCExtImageCopyCaptureSessionV1* r, uint32_t width, uint32_t height) {
        pendingConstraints.width   = width;
        pendingConstraints.height  = height;
        pendingConstraints.hasSize = true;
    });
    pCaptureSession->setShmFormat([this](CCExtImageCopyCaptureSessionV1* r, uint32_t format) { pendingConstraints.shmFormats.emplace_back(format); });
    pCaptureSession->setDone([this](CCExtImageCopyCaptureSessionV1* r) {
        if (!pendingConstraints.hasSize || pendingConstraints.width == 0 || pendingConstraints.height == 0) {
            Debug::log(CRIT, "Image copy capture provided invalid buffer constraints for output %s", name.c_str());
            g_pHyprpicker->finish(1);
        }

        constraints        = std::move(pendingConstraints);
        pendingConstraints = {};
        ++constraintsGeneration;

        if (!selectCaptureFormat(constraints.shmFormats)) {
            Debug::log(CRIT, "Image copy capture did not provide a supported SHM format for output %s", name.c_str());
            g_pHyprpicker->finish(1);
        }

        if (!pCaptureFrame)
            capture();
    });
    pCaptureSession->setStopped([this](CCExtImageCopyCaptureSessionV1* r) {
        Debug::log(CRIT, "Image copy capture session stopped for output %s", name.c_str());
        g_pHyprpicker->finish(1);
    });
}

void SMonitor::capture() {
    const auto FORMAT = selectCaptureFormat(constraints.shmFormats);
    if (!FORMAT) {
        Debug::log(CRIT, "Image copy capture has no usable SHM format for output %s", name.c_str());
        g_pHyprpicker->finish(1);
    }

    constexpr uint64_t MAX_WAYLAND_SIZE = std::numeric_limits<int32_t>::max();
    const uint64_t     stride           = static_cast<uint64_t>(constraints.width) * FORMAT->bytesPerPixel;
    const uint64_t     normalizedStride = static_cast<uint64_t>(constraints.width) * 4;
    if (constraints.width > MAX_WAYLAND_SIZE || constraints.height > MAX_WAYLAND_SIZE || stride > MAX_WAYLAND_SIZE || normalizedStride > MAX_WAYLAND_SIZE ||
        stride * constraints.height > MAX_WAYLAND_SIZE) {
        Debug::log(CRIT, "Image copy capture provided an oversized buffer for output %s", name.c_str());
        g_pHyprpicker->finish(1);
    }

    captureBytesPerPixel       = FORMAT->bytesPerPixel;
    frameConstraintsGeneration = constraintsGeneration;
    pLS->screenBuffer =
        makeShared<SPoolBuffer>(Vector2D{static_cast<double>(constraints.width), static_cast<double>(constraints.height)}, FORMAT->format, static_cast<uint32_t>(stride));
    pCaptureFrame = makeShared<CCExtImageCopyCaptureFrameV1>(pCaptureSession->sendCreateFrame());

    pCaptureFrame->setReady([this](CCExtImageCopyCaptureFrameV1* r) { captureReady(); });
    pCaptureFrame->setFailed([this](CCExtImageCopyCaptureFrameV1* r, extImageCopyCaptureFrameV1FailureReason reason) { captureFailed(reason); });

    pCaptureFrame->sendAttachBuffer(pLS->screenBuffer->buffer->resource());
    pCaptureFrame->sendDamageBuffer(0, 0, static_cast<int32_t>(constraints.width), static_cast<int32_t>(constraints.height));
    pCaptureFrame->sendCapture();
}

void SMonitor::captureReady() {
    if (!pLS->screenBuffer) {
        Debug::log(CRIT, "Image copy capture returned without a buffer for output %s", name.c_str());
        g_pHyprpicker->finish(1);
    }

    void* data = pLS->screenBuffer->data;
    if (captureBytesPerPixel == 4)
        g_pHyprpicker->convertBuffer(pLS->screenBuffer);
    else if (captureBytesPerPixel == 3) {
        data                          = g_pHyprpicker->convert24To32Buffer(pLS->screenBuffer);
        pLS->screenBuffer->paddedData = data;
    } else {
        Debug::log(CRIT, "Unsupported image copy capture pixel size %u", captureBytesPerPixel);
        g_pHyprpicker->finish(1);
    }

    const auto CAIROSTRIDE = static_cast<int32_t>(pLS->screenBuffer->pixelSize.x) * 4;
    pLS->screenBuffer->surface =
        cairo_image_surface_create_for_data(static_cast<unsigned char*>(data), CAIRO_FORMAT_ARGB32, pLS->screenBuffer->pixelSize.x, pLS->screenBuffer->pixelSize.y, CAIROSTRIDE);
    if (cairo_surface_status(pLS->screenBuffer->surface) != CAIRO_STATUS_SUCCESS) {
        Debug::log(CRIT, "Failed to create a Cairo surface for captured output %s", name.c_str());
        g_pHyprpicker->finish(1);
    }

    Debug::log(TRACE, "Image copy frame ready: pixel %.0fx%.0f", pLS->screenBuffer->pixelSize.x, pLS->screenBuffer->pixelSize.y);

    g_pHyprpicker->recheckACK();
    g_pHyprpicker->renderSurface(pLS);

    pCaptureFrame.reset();
    pCaptureSession.reset();
    pCaptureSource.reset();
}

void SMonitor::captureFailed(extImageCopyCaptureFrameV1FailureReason reason) {
    pCaptureFrame.reset();
    pLS->screenBuffer.reset();

    if (reason == EXT_IMAGE_COPY_CAPTURE_FRAME_V1_FAILURE_REASON_BUFFER_CONSTRAINTS) {
        if (constraintsGeneration > frameConstraintsGeneration)
            capture();
        else
            Debug::log(WARN, "Image copy capture constraints changed for output %s, waiting for a new constraint batch", name.c_str());
        return;
    }

    if (reason == EXT_IMAGE_COPY_CAPTURE_FRAME_V1_FAILURE_REASON_UNKNOWN && unknownFailureCount++ == 0) {
        Debug::log(WARN, "Image copy capture failed for output %s, retrying", name.c_str());
        capture();
        return;
    }

    if (reason == EXT_IMAGE_COPY_CAPTURE_FRAME_V1_FAILURE_REASON_STOPPED)
        Debug::log(CRIT, "Image copy capture session stopped for output %s", name.c_str());
    else
        Debug::log(CRIT, "Image copy capture failed for output %s", name.c_str());

    g_pHyprpicker->finish(1);
}
