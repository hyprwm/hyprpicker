#pragma once

#include "../defines.hpp"

struct SPoolBuffer {
    SPoolBuffer(const Vector2D& size, uint32_t format, uint32_t stride);
    ~SPoolBuffer();

    SP<CCWlBuffer>   buffer  = nullptr;
    cairo_surface_t* surface = nullptr;
    cairo_t*         cairo   = nullptr;
    void*            data    = nullptr;

    // malloc'ed buffer for 24bit formats
    void*       paddedData = nullptr;

    size_t      size   = 0;
    uint32_t    stride = 0;
    Vector2D    pixelSize;

    uint32_t    format;

    std::string name;

    bool        busy = false;
};