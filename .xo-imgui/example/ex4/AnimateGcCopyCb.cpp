/* AnimateGcCopyCb.cpp */

#include "AnimateGcCopyCb.hpp"

void
AnimateGcCopyCb::notify_gc_copy(std::size_t z,
                                const void * src_addr,
                                const void * dest_addr,
                                generation src_gen,
                                generation dest_gen)
{
    using xo::scope;
    using xo::xtag;
    using xo::gc::generation_result;
    using xo::gc::generation;
    using xo::gc::role;

    scope log(XO_DEBUG(false),
              xtag("z", z),
              xtag("src", src_addr),
              xtag("dest", dest_addr),
              xtag("src_gen", src_gen),
              xtag("dest_gen", dest_gen));

    auto [src_gen2, src_offset, src_alloc, src_size] = p_app_state_->gc_->fromspace_location_of(src_addr);

    if (src_gen2 == generation_result::not_found) {
        auto [lo, hi] = p_app_state_->gc_->nursery_span(role::from_space);

        log && log(xtag("N.from.lo", (void*)lo), xtag("N.from.hi", (void*)hi));

        assert(false);
    }

    generation src_valid_gen = xo::gc::valid_genresult2gen(src_gen2);

    auto [dest_gen2, dest_offset, _, dest_size] = p_app_state_->gc_->tospace_location_of(dest_addr);

    generation dest_valid_gen = xo::gc::valid_genresult2gen(dest_gen2);

    p_app_state_->copy_detail_v_.push_back(GcCopyDetail(z,
                                                        src_valid_gen, src_offset, src_alloc,
                                                        dest_valid_gen, dest_offset, dest_size));

    if (dest_valid_gen == generation::nursery) {
        p_app_state_->copy_detail_max_nursery_dest_offset_
            = std::max(p_app_state_->copy_detail_max_nursery_dest_offset_, dest_offset);
        p_app_state_->copy_detail_nursery_dest_size_
            = std::max(p_app_state_->copy_detail_nursery_dest_size_, dest_size);
    } else if (dest_valid_gen == generation::tenured) {
        p_app_state_->copy_detail_max_tenured_dest_offset_
            = std::max(p_app_state_->copy_detail_max_tenured_dest_offset_, dest_offset);
        p_app_state_->copy_detail_tenured_dest_size_
            = std::max(p_app_state_->copy_detail_tenured_dest_size_, dest_size);
    }

    /* will be animated across frames, see animate_gc_copy() */
}

/* AnimateGcCopyCb.cp */
