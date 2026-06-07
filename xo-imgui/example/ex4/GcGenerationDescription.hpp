/* GcGenerationDescription.hpp */

#pragma once

#include "xo/alloc/GcStatistics.hpp"
#include <cstdint>

/* We need GUI to be able to fall behind true GC state, so we can animate transitions.
 * To help make this work, provide a model for GC state sufficient to drive rendering.
 */
struct GcGenerationDescription {
    using generation = xo::gc::generation;

    GcGenerationDescription() = default;
    GcGenerationDescription(generation gen,
                            const char * name, const char * mnemonic, const char * gc_type,
                            std::uint8_t polarity,
                            std::size_t tospace_scale,
                            std::size_t before_ckp, std::size_t after_ckp,
                            std::size_t reserved, std::size_t committed, std::size_t gc_threshold)
        : name_{name}, mnemonic_{mnemonic}, gc_type_{gc_type}, polarity_{polarity},
          tospace_scale_{tospace_scale},
          before_checkpoint_{before_ckp}, after_checkpoint_{after_ckp},
          reserved_{reserved}, committed_{committed},
          gc_threshold_{gc_threshold} {}

    /** scale (in bytes) for drawing space **/
    std::size_t scale() const { return std::max(committed_, gc_threshold_); }

    /** nursery or tenured **/
    generation generation_;

    /** "nursery" or "tenured" **/
    const char * name_ = nullptr;

    /** "N" or "T" **/
    const char * mnemonic_ = nullptr;

    /** "incremental" or "full" **/
    const char * gc_type_ = nullptr;

    /** alternates between {0, 1} on each GC **/
    std::uint8_t polarity_ = 0;

    /** size of to-space in bytes represented on screen.
     *  (note however when we animate GC, space roles have already reversed,
     *   so then this will refer to old to-space = new from-space)
     **/
    std::size_t tospace_scale_ = 0;

    std::size_t before_checkpoint_ = 0;
    std::size_t after_checkpoint_ = 0;
    std::size_t reserved_ = 0;
    std::size_t committed_ = 0;
    // G_to_gc_threshold = G1_to_size + gc->config().incr_gc_threshold_;
    std::size_t gc_threshold_ = 0;
};

/* end GcGenerationDescription.hpp */
