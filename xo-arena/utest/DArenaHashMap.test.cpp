/** @file DArenaHashMap.test.cpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DArenaHashMap.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::DArenaHashMap;
    //using xo::mM::ArenaConfig;

    namespace ut {
        TEST_CASE("DArenaHashMap-ctor", "[arena][DArenaHashMap]")
        {
            using HashMap = DArenaHashMap<int, int>;

            HashMap map;

            REQUIRE(map.empty());
            REQUIRE(map.capacity() == HashMap::group_size());
        }
    }
}

/* end DArenaHashMap.test.cpp */
