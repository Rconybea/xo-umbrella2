/** @file MutationLogEntry.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "MutationLogEntry.hpp"
#include "GCObjectStore.hpp"
#include "MutationLogStatistics.hpp"

namespace xo {
    using xo::mm::MutationLogStatistics;
    using xo::reflect::typeseq;

    namespace mm {

        MutationLogEntry::MutationLogEntry(void * parent,
                                           void ** p_data,
                                           obj<AGCObject> snap)
                : parent_{parent},
                  p_data_{p_data},
                  snap_{snap}
        {}

        bool
        MutationLogEntry::check_forward_inplace(GCObjectStore * gcos,
                                                MutationLogStatistics * p_counters) noexcept
        {
            /** Several cases here based on state of {mlog entry, parent}.
             *
             *  1. gc already updated P->P', C->C',
             *     still have snap->P, snap->C
             *     update P'->C', snap->P', snap->C'
             *  2. gc already updated P->C to P->C'.
             *     still have snap->C. mlog entry is current.
             *     update snap->C to snap->C'
             *  3. gc has not updated P->C,
             *     but it has (independently) evacuated C to C'.
             *     still have snap->C, P->C. mlog entry is current.
             *     update P->C' and snap->C'
             *  4. gc has not moved P or C. mlog entry is up to date.
             *     no update. P,C may yet be rescued, or may be garbage.
             *  5. P->D, D not in {C,C'}
             *     mlog entry has been superseded.
             *     Be careful of confusing this case with 1..4
             **/

            // if either of {parent, snapshot} has been forwarded,
            // update with destination.
            //
            // Don't do this with child, since child might now point
            // outside gc-owned space

            bool upd_flag = this->check_forward_parent_inplace(gcos, p_counters);

            AllocInfo snap_info = gcos->alloc_info((std::byte *)snap_.data_);

            if (snap_info.is_forwarding_tseq()) {
                void * snap_from = snap_.data_;
                void * snap_to = *(void **)snap_.data_;

                if (snap_from == *p_data_) {
                    // parent still refers to forwarding pointer, needs fix
                    *p_data_ = snap_to;
                    // also fix snapshot
                    this->snap_.reset_opaque(snap_to);

                    upd_flag = true;
                } else if (snap_to == *p_data_) {
                    // parent updated, but snapshot stale
                    this->snap_.reset_opaque(snap_to);

                    upd_flag = true;
                } else {
                    // superseded mlog entry
                }
            }

            return upd_flag;
        }

        bool
        MutationLogEntry::check_forward_parent_inplace(GCObjectStore * gcos,
                                                       MutationLogStatistics * p_counters) noexcept
        {
            AllocInfo parent_info = gcos->alloc_info((std::byte *)parent_);

            if (parent_info.is_forwarding_tseq()) {
                void * parent_to = *(void **)parent_;

                std::size_t offset
                    = (std::byte *)p_data_ - (std::byte *)parent_;

                void ** p_data_to = (void **)((std::byte *)parent_to + offset);

                this->parent_ = parent_to;
                this->p_data_ = p_data_to;

                ++(p_counters->n_live_parent_);

                return true;
            } else {
                return false;
            }
        }

        bool
        MutationLogEntry::refresh_snapshot(Generation parent_gen,
                                           GCObjectStore * gcos) noexcept
        {
            scope log(XO_DEBUG(gcos->config().debug_flag_));

            void * child_data = *p_data_;

            Generation child_gen_to
                = gcos->generation_of(Role::to_space(), child_data);

            if (child_gen_to.is_sentinel()) {
                log && log("child not in to-space");

                // unreachable, since:
                // 1. not in from-space, if it were forwarded,
                //    caller would already have forwarded this mlog entry.
                // 2. caller already confirmed mlog entry current.
                // 3. would not create mlog entry for non-gc-owned child
                // in any case, would be legal to discard mlog entry here.

                assert(false);

                snap_.data_ = nullptr; // hygiene

                return false;
            }

            const GCObjectStoreConfig & config = gcos->config();

            bool need_mlog_entry
                = ((child_gen_to + 1 < config.n_generation_)
                   && (config.promotion_threshold(parent_gen)
                       > config.promotion_threshold(child_gen_to)));

            if (need_mlog_entry) {
                AllocInfo child_info = gcos->alloc_info((std::byte*)child_data);

                assert(!child_info.is_forwarding_tseq());

                log && log("need mlog entry", xtag("tseq", child_info.tseq()));

                AGCObject * iface = gcos->lookup_type(typeseq(child_info.tseq()));

                if (iface) {
                    this->snap_ = obj<AGCObject>(iface, child_data);

                    // snapshot updated, keep mlog entry
                    return true;
                } else {
                    log && log("facet install error!");

                    // facet install error

                    assert(false);

                    return false;
                }
            } else {
                log && log("retire mlog entry");

                // retire this entry.
                return false;
            }
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end MutationLogEntry.cpp */
