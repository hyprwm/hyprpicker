#pragma once

#include "../defines.hpp"
#include <hyprutils/math/Vector2D.hpp>
using namespace Hyprutils::Math;

class CLayerSurface;

struct SMonitor {
    SMonitor(SP<CCWlOutput> output_);
    void                               initCapture();

    std::string                        name         = "";
    SP<CCWlOutput>                     output       = nullptr;
    uint32_t                           wayland_name = 0;
    Vector2D                           size;
    int32_t                            scale = 1;

    bool                               ready = false;

    CLayerSurface*                     pLS = nullptr;
    SP<CCExtImageCaptureSourceV1>      pCaptureSource;
    SP<CCExtImageCopyCaptureSessionV1> pCaptureSession;
    SP<CCExtImageCopyCaptureFrameV1>   pCaptureFrame;

  private:
    struct SCaptureConstraints {
        uint32_t              width   = 0;
        uint32_t              height  = 0;
        bool                  hasSize = false;
        std::vector<uint32_t> shmFormats;
    };

    void                capture();
    void                captureReady();
    void                captureFailed(extImageCopyCaptureFrameV1FailureReason reason);

    SCaptureConstraints pendingConstraints;
    SCaptureConstraints constraints;
    uint64_t            constraintsGeneration      = 0;
    uint64_t            frameConstraintsGeneration = 0;
    uint32_t            captureBytesPerPixel       = 0;
    uint32_t            unknownFailureCount        = 0;
};
