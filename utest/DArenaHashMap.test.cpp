/** @file DArenaHashMap.test.cpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DArenaHashMap.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::DArenaHashMapUtil;
    using xo::mm::DArenaHashMap;
    //using xo::mM::ArenaConfig;

    namespace ut {
        TEST_CASE("DArenaHashMap-ctor", "[arena][DArenaHashMap]")
        {
            using HashMap = DArenaHashMap<int, int>;

            HashMap map;

            REQUIRE(map.empty());
            REQUIRE(map.size() == 0);
            REQUIRE(map.groups() == 1);
            REQUIRE(map.capacity() == DArenaHashMapUtil::c_group_size);
        }

        TEST_CASE("DArenaHashMap-ctor2", "[arena][DArenaHashMap]")
        {
            using HashMap = DArenaHashMap<int, int>;

            HashMap map(257);

            REQUIRE(map.empty());
            REQUIRE(map.size() == 0);
            REQUIRE(map.capacity() == map.groups() * DArenaHashMapUtil::c_group_size);
            REQUIRE(map.capacity() == std::max(512ul,
                                               DArenaHashMapUtil::c_group_size));
        }
    }
}

/* end DArenaHashMap.test.cpp */
