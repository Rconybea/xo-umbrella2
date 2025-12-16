/** @file random_allocs.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "random_allocs.hpp"
#include <xo/indentlog/scope.hpp>
#include <xo/indentlog/print/tag.hpp>
#include <catch2/catch.hpp>
#include <map>

namespace utest {
    using xo::rng::xoshiro256ss;
    using xo::facet::obj;
    using xo::scope;
    using xo::xtag;
    using std::uint32_t;
    using std::byte;

    /* remember an allocation result.
     * application owns memory in [lo, lo+z)
     */
    struct AllocInfo {
        AllocInfo() = default;
        AllocInfo(byte * lo, size_t z) : lo_{lo}, z_{z} {}

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
         * allocs sorted on AllocInfo::lo
         */
        std::map<byte *, AllocInfo> allocs_by_lo_map;
        /* allocs sorted on AllocInfo::hi */
        std::map<byte *, AllocInfo*> allocs_by_hi_map;

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

            allocs_by_lo_map[mem] = AllocInfo(mem, z);
            allocs_by_hi_map[mem + z] = &(allocs_by_lo_map[mem]);


        }

        return true;
    }
}

/* end random_allocs.cpp */
