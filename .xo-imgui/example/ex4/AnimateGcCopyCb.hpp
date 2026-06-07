/* AnimateGcCopyCb.hpp */

#pragma once

#include "xo/alloc/GC.hpp"
#include "AppState.hpp"
#include "DrawState.hpp"

struct DrawState;

struct AnimateGcCopyCb : public xo::gc::GcCopyCallback {
    using generation = xo::gc::generation;

    explicit AnimateGcCopyCb(AppState * appstate, DrawState * drawstate)
        : p_app_state_{appstate}, p_draw_state_{drawstate} {}

    virtual void notify_gc_copy(std::size_t z,
                                const void * src_addr, const void * dest_addr,
                                generation src_gen, generation dest_gen);

    AppState * p_app_state_ = nullptr;
    DrawState * p_draw_state_ = nullptr;
};

/* end AnimateGcCopyCb.hpp */
