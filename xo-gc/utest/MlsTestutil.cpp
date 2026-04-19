/** @file MlsTestutil.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include "MlsTestutil.hpp"
#include <catch2/catch.hpp>

namespace ut {
    using xo::mm::GCObjectStore;
    using xo::mm::MutationLog;
    using xo::mm::MutationLogEntry;
    using xo::mm::AllocInfo;
    using xo::mm::Role;
    using xo::mm::Generation;
    using xo::mm::c_max_generation;

    void
    MlsTestutil::verify_fromspace_only_logged(const MutationLogStore & mls,
                                              Generation upto)
    {
        for (Generation gi{0}; gi < std::min(upto, Generation(c_max_generation - 1)); ++gi) {
            // from-space mlog may be empty or not

            // after swapping roles at beginning of GC,
            // to-space mlog must be empty
            {
                const MutationLog * mlog = mls.get_mlog(Role::to_space(), gi);
                REQUIRE(mlog->empty());
            }

            // triage mlog must be empty at beginning of GC phase
            {
                const MutationLog * mlog = mls.triage_mlog(gi);
                REQUIRE(mlog->empty());
            }
        }
    }

    void
    MlsTestutil::verify_tospace_only_logged(const MutationLogStore & mls,
                                            Generation upto)
    {
        for (Generation gi{0}; gi < std::min(upto, Generation(c_max_generation - 1)); ++gi) {
            // to-space mlog may be empty or not

            // from-space mlog must be empty in all generations.
            // (only non-empty in GC phase, before GC completes)
            {
                const MutationLog * mlog = mls.get_mlog(Role::from_space(), gi);
                REQUIRE(mlog->empty());
            }

            // traige mlog must be empty in all generations
            // (only non-empty in GC phase, before GC completes)
            {
                const MutationLog * mlog = mls.triage_mlog(gi);
                REQUIRE(mlog->empty());
            }
        }
    }

    void
    MlsTestutil::verify_mlog_load_bearing(const MutationLogStore & mls,
                                          Generation upto)
    {
        // reminders:
        // - pointers from non-gc-owned objects permitted only from root objects.
        //   Such source objects are visited on every collection and don't need (or get)
        //   mlog entries. Exclude from consideration here.
        // - Similarly pointers to non-gco-owned objects also don't need mlog entries.

        const GCObjectStore & gcos = *mls.gco_store_;

        for (Generation gi{0}; gi < std::min(upto, Generation(c_max_generation - 1)); ++gi) {

            const MutationLog * mlog = mls.get_mlog(Role::to_space(), gi);

            for (const MutationLogEntry & entry : *mlog) {
                REQUIRE(entry.parent());
                REQUIRE(entry.p_data());
                REQUIRE(entry.snap());

                if (entry.is_active()) {
                    AllocInfo src_info = gcos.alloc_info((std::byte *)entry.parent());
                    void * dest = *entry.p_data();
                    AllocInfo dest_info = gcos.alloc_info((std::byte *)*entry.p_data());

                    // source and destination must both be in to-space
                    REQUIRE(gcos.contains_allocated(Role::to_space(), entry.parent()));
                    REQUIRE(gcos.contains_allocated(Role::to_space(), *entry.p_data()));

                    // either:
                    // 1. source in older generation than destination,
                    //    (so destination may move under incremental collection,
                    //     while parent generation stays put)
                    // 2. source may eventually promote to older generation,
                    //    before destination.
                    //
                    // otherwise pointer does not require and should not have
                    // a mutation log entry
                    //
                    REQUIRE(src_info.age() > dest_info.age());
                }
            }
        }
    }
} /*namespace ut*/

/* end MlsTestutil.cpp */
