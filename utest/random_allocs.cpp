/** @file random_allocs.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "random_allocs.hpp"
#include "arena/DArena.hpp"
#include "padding.hpp"
#include <xo/indentlog/scope.hpp>
#include <xo/indentlog/print/tag.hpp>
#include <catch2/catch.hpp>
#include <map>

namespace utest {
    using xo::mm::AllocInfo;
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;
    using xo::mm::padding;
    using xo::rng::xoshiro256ss;
    using xo::facet::obj;
    using xo::scope;
    using xo::xtag;
    using std::uint32_t;
    using std::byte;

    /* remember an allocation result.
     * application owns memory in [lo, lo+z)
     */
    struct Alloc {
        Alloc() = default;
        Alloc(byte * lo, size_t z) : lo_{lo}, z_{z} {}

        byte * lo() const { return lo_; }
        byte * hi() const { return lo_ + z_; }

        byte * lo_ = nullptr;
        size_t z_ = 0;
    };

    bool
    AllocUtil::random_allocs(uint32_t n_alloc,
                             bool catch_flag,
                             xoshiro256ss * p_rgen,
                             obj<AAllocator> mm)
    {
        scope log(XO_DEBUG(catch_flag), xtag("n-alloc", n_alloc));

        /* track allocs. verify:
         *  - allocs are non-overlapping
         *  - allocs have valid alloc header
         *  - allocs surrounded by guard bytes
         *
         * allocs sorted on Alloc::lo
         */
        std::map<byte *, Alloc> allocs_by_lo_map;
        /* allocs sorted on Alloc::hi */
        std::map<byte *, Alloc*> allocs_by_hi_map;

        for (uint32_t i_alloc = 0; i_alloc < n_alloc; ++i_alloc) {
            std::normal_distribution<double> ngen{5.0, 1.5};

            double si = ngen(*p_rgen);
            double zi = ::pow(2.0, si);
            std::size_t z = ::ceil(zi);

            bool ok_flag = true;

            std::byte * mem = mm.alloc(z);
            log && log(xtag("i_alloc", i_alloc),
                       xtag("si", si),
                       xtag("zi", zi),
                       xtag("mem", mem));
            log && log(xtag("used", mm.allocated()),
                       xtag("avail", mm.available()),
                       xtag("commit", mm.committed()),
                       xtag("resv", mm.reserved()));


            REQUIRE_ORFAIL(ok_flag, catch_flag, mem != nullptr);
            REQUIRE_ORFAIL(ok_flag, catch_flag, mm.contains(mem));
            REQUIRE_ORFAIL(ok_flag, catch_flag, mm.last_error().error_seq_ == 0);
            REQUIRE_ORFAIL(ok_flag, catch_flag, mm.last_error().error_ == xo::mm::error::none);

            {
                auto ix = allocs_by_lo_map.lower_bound(mem);
                if (ix != allocs_by_lo_map.end()) {
                    REQUIRE_ORFAIL(ok_flag, catch_flag, (ix->first > mem + z));
                }
            }

            {
                auto ix = allocs_by_hi_map.upper_bound(mem);
                if (ix != allocs_by_hi_map.end()) {
                    --ix;
                    REQUIRE_ORFAIL(ok_flag, catch_flag, (ix->first < mem));
                }
            }

            allocs_by_lo_map[mem] = Alloc(mem, z);
            allocs_by_hi_map[mem + z] = &(allocs_by_lo_map[mem]);

            /* verify we can recover alloc info */
            AllocInfo info = mm.alloc_info(mem);

            REQUIRE_ORFAIL(ok_flag, catch_flag, info.is_valid());

            REQUIRE_ORFAIL(ok_flag, catch_flag,
                           info.size() == padding::with_padding(z));

            /* age isn't configured -> 0 = sentinel */
            REQUIRE_ORFAIL(ok_flag, catch_flag, info.age() == 0);
            /* tseq isn't configured -> 0 = sentinel */
            REQUIRE_ORFAIL(ok_flag, catch_flag, info.tseq() == 0);

            if ((info.p_config_->guard_z_ > 0)
                || info.guard_lo().first
                || info.guard_lo().second
                || info.guard_hi().first
                || info.guard_hi().second)
            {
                REQUIRE_ORFAIL(ok_flag, catch_flag,
                               info.guard_lo().first != nullptr);
                REQUIRE_ORFAIL(ok_flag, catch_flag,
                               info.guard_lo().second != nullptr);
                REQUIRE_ORFAIL(ok_flag, catch_flag,
                               info.guard_lo().first + info.guard_z()
                               == info.guard_lo().second);

                for (const byte * p = info.guard_lo().first;
                     p != info.guard_lo().second; ++p)
                {
                    REQUIRE_ORFAIL(ok_flag, catch_flag, (char)*p == info.guard_byte());
                }

                REQUIRE_ORFAIL(ok_flag, catch_flag,
                               info.guard_hi().first != nullptr);
                REQUIRE_ORFAIL(ok_flag, catch_flag,
                               info.guard_hi().second != nullptr);
                REQUIRE_ORFAIL(ok_flag, catch_flag,
                               info.guard_hi().first + info.guard_z()
                               == info.guard_hi().second);

                for (const byte * p = info.guard_hi().first;
                     p != info.guard_hi().second; ++p)
                {
                    REQUIRE_ORFAIL(ok_flag, catch_flag, (char)*p == info.guard_byte());
                }


            } else {
                /* control here only if all of:
                 *  - guard_z is zero
                 *  - guard_lo empty
                 *  - guard_hi empty
                 */

                REQUIRE_ORFAIL(ok_flag, catch_flag,
                               info.guard_lo().first == nullptr);
                REQUIRE_ORFAIL(ok_flag, catch_flag,
                               info.guard_lo().second == nullptr);

                REQUIRE_ORFAIL(ok_flag, catch_flag,
                               info.guard_hi().first == nullptr);
                REQUIRE_ORFAIL(ok_flag, catch_flag,
                               info.guard_hi().second == nullptr);

            }

            /** scratch arena for iterators **/
            DArena scratch_mm = DArena::map(ArenaConfig{.name_ = "scratch",
                                                        .size_ = 4*1024,
                                                        .hugepage_z_ = 4*1024 });
            auto range = mm.alloc_range(scratch_mm);

#ifdef NOT_YET // to verify iteration here, need iterator support in AAllocator

            /* verify iteration visits all the allocs, exactly once */
            {
                auto alloc_map = allocs_by_lo_map;

                for (AllocInfo info : mm) {
                }

            }
#endif

        }

        return true;
    }
}

/* end random_allocs.cpp */
