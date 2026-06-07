/* GcStateDescription.hpp */

#pragma once

#include "GcGenerationDescription.hpp"

/* We need GUI to be able to fall behind true GC state, so we can animate transitions.
 * To help make this work, provide a model for GC state sufficient to drive rendering.
 */
struct GcStateDescription {
    using generation = xo::gc::generation;

    GcStateDescription() = default;
    GcStateDescription(const GcGenerationDescription & nursery,
                       const GcGenerationDescription & tenured,
                       std::size_t gc_size,
                       std::size_t gc_committed,
                       std::size_t gc_allocated,
                       std::size_t gc_available,
                       std::size_t gc_mlog_size,
                       std::size_t total_promoted,
                       std::size_t total_n_mutation
        );

    const GcGenerationDescription & get_gendescr(generation g) const { return gen_state_v_[gen2int(g)]; }

    std::array<GcGenerationDescription, static_cast<std::size_t>(generation::N)> gen_state_v_;

    /** see @ref GC::size **/
    std::size_t gc_size_ = 0;
    /** see @ref GC::committed **/
    std::size_t gc_committed_ = 0;
    /** see @ref GC::allocated **/
    std::size_t gc_allocated_ = 0;
    /** see @ref GC::available **/
    std::size_t gc_available_ = 0;
    /** see @ref GC::mlog_size **/
    std::size_t gc_mlog_size_ = 0;

    /** see @ref GcStatistics::total_promoted_ **/
    std::size_t total_promoted_ = 0;
    /** see @ref GcStatistics::n_mutation_ **/
    std::size_t total_n_mutation_ = 0;
};

/* end GcStateDescription.hpp */
