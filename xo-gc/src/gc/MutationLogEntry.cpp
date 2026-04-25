/** @file MutationLogEntry.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "MutationLogEntry.hpp"
#include <xo/gc/GCObjectStore.hpp>

namespace xo {
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
        MutationLogEntry::refresh_snapshot(Generation parent_gen,
                                           GCObjectStore * gcos) noexcept
        {
            void * child_data = *p_data_;
            // note: never the same child_info as computed at the top of
            //       MutationLogEntry._preserve_child_of_live_parent()
            //       Child pointer was either forwarding pointer or moved.
            //       In either case must pickup info for new location.
            //
            AllocInfo child_info = gcos->alloc_info((std::byte*)child_data);

            if (child_info.is_forwarding_tseq()) {
                // code salvaged from MutationLogStore._check_keep_mutation_aux()
                // as reminder.  But if caller is gc will have to know this anyway,
                // so it can move child

                // if (info.is_forwarding_tseq()) {
                //    child_data = *(void **)child_data;
                //    info = gcos->alloc_info((std::byte *)child_data);
                //}

                assert(false);  // for now assuming caller forward child
            }

            Generation child_gen_to
                = gcos->generation_of(Role::to_space(), child_data);

            if (child_gen_to.is_sentinel()) {
                // child no longer points to gc-owned space.
                // 1. may not have an alloc header (could be a static global for example),
                //    so AllocInfo not available
                // 2. doesn't need a mutation log entry since this gc can't move destination

                snap_.data_ = nullptr; // hygiene

                return false;
            }

            const GCObjectStoreConfig & config = gcos->config();

            bool need_mlog_entry
                = ((child_gen_to + 1 < config.n_generation_)
                   && (config.promotion_threshold(parent_gen)
                       > config.promotion_threshold(child_gen_to)));

            if (need_mlog_entry) {
                AGCObject * iface = gcos->lookup_type(typeseq(child_info.tseq()));

                if (iface) {
                    this->snap_ = obj<AGCObject>(iface, child_data);

                    // snapshot updated, keep mlog entry
                    return true;
                } else {
                    assert(false);

                    return false;
                }
            } else {
                // retire this entry.
                return false;
            }
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end MutationLogEntry.cpp */
