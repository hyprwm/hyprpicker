#pragma once

#include "../defines.hpp"

namespace Events {
    void geometry(void *data, wl_output *output, int32_t x, int32_t y, int32_t width_mm, int32_t height_mm, int32_t subpixel, const char *make, const char *model, int32_t transform);

    void mode(void *data, wl_output *output, uint32_t flags, int32_t width, int32_t height, int32_t refresh);

    void done(void *data, wl_output *wl_output);

    void scale(void *data, wl_output *wl_output, int32_t scale);

    void name(void *data, wl_output *wl_output, const char *name);

    void description(void *data, wl_output *wl_output, const char *description);

    void ls_configure(void *data, zwlr_layer_surface_v1 *surface, uint32_t serial, uint32_t width, uint32_t height);

    void handleGlobal(void *data, wl_registry *registry, uint32_t name, const char *interface, uint32_t version);

    void handleGlobalRemove(void *data, wl_registry *registry, uint32_t name);

    void handleCapabilities(void *data, wl_seat *wl_seat, uint32_t capabilities);

    void handlePointerMotion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y);

    void handlePointerButton(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t button_state);

    void handlePointerAxis(void *data, wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value);

    void handlePointerEnter(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y);

    void handlePointerLeave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface);

    void handleFrameDone(void *data, struct wl_callback *callback, uint32_t time);

    void handleBufferRelease(void *data, struct wl_buffer *wl_buffer);

    void handleSCBuffer(void *data, struct zwlr_screencopy_frame_v1 *frame, uint32_t format, uint32_t width, uint32_t height, uint32_t stride);

    void handleSCFlags(void *data, struct zwlr_screencopy_frame_v1 *frame, uint32_t flags);

    void handleSCReady(void *data, struct zwlr_screencopy_frame_v1 *frame, uint32_t tv_sec_hi, uint32_t tv_sec_lo, uint32_t tv_nsec);

    void handleSCFailed(void *data, struct zwlr_screencopy_frame_v1 *frame);

    inline const wl_output_listener outputListener = {.geometry = geometry, .mode = mode, .done = done, .scale = scale, .name = name, .description = description};

    inline const zwlr_layer_surface_v1_listener layersurfaceListener = { .configure = ls_configure };

    inline const wl_registry_listener registryListener = { .global = handleGlobal, .global_remove = handleGlobalRemove };
    
    inline const wl_seat_listener seatListener = { .capabilities = handleCapabilities };

    inline const wl_pointer_listener pointerListener = { .enter = handlePointerEnter, .leave = handlePointerLeave, .motion = handlePointerMotion, .button = handlePointerButton, .axis = handlePointerAxis };

    inline const wl_callback_listener frameListener = { .done = handleFrameDone };

    inline const wl_buffer_listener bufferListener = { .release = handleBufferRelease };

    inline const zwlr_screencopy_frame_v1_listener screencopyListener = { .buffer = handleSCBuffer, .flags = handleSCFlags, .ready = handleSCReady, .failed = handleSCFailed };
};