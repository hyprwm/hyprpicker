#include "PoolBuffer.hpp"
#include "../hyprpicker.hpp"

SPoolBuffer::SPoolBuffer(const Vector2D& pixelSize_, uint32_t format_, uint32_t stride_) : stride(stride_), pixelSize(pixelSize_), format(format_) {
    const size_t SIZE = stride * pixelSize.y;

    const auto   FD = g_pHyprpicker->createPoolFile(SIZE, name);

    if (FD == -1) {
        Debug::log(CRIT, "Unable to create pool file!");
        g_pHyprpicker->finish(1);
    }

    const auto DATA = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, FD, 0);

    size = SIZE;
    data = DATA;

    auto POOL = makeShared<CCWlShmPool>(g_pHyprpicker->m_pSHM->sendCreatePool(FD, SIZE));
    buffer    = makeShared<CCWlBuffer>(POOL->sendCreateBuffer(0, pixelSize.x, pixelSize.y, stride, format));

    buffer->setRelease([this](CCWlBuffer* r) { busy = false; });

    POOL.reset();

    close(FD);
}

SPoolBuffer::~SPoolBuffer() {
    buffer.reset();
    cairo_destroy(cairo);
    cairo_surface_destroy(surface);
    munmap(data, size);

    cairo   = nullptr;
    surface = nullptr;

    unlink(name.c_str());

    if (paddedData)
        free(paddedData);
}