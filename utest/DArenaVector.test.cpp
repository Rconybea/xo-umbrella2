/** @file DArenaVector.test.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "xo/arena/DArenaVector.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::DArenaVector;
    using xo::mm::ArenaConfig;
    using std::byte;

    namespace ut {
        TEST_CASE("DArenaVector-tiny", "[arena][DArenaVector]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 1 };
            DArenaVector<double> arenavec = DArenaVector<double>::map(cfg);

            REQUIRE(arenavec.empty());
        }
    }
}

/* end DArenaVector.test.cpp */
