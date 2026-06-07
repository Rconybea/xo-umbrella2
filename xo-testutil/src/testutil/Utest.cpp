/** @file Utest.cpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#include "Utest.hpp"
#include "UtestConfig.hpp"
#include <catch2/catch.hpp>

namespace xo {
    scope
    Utest::ut_scope() {
        return scope(XO_DEBUG(UtestConfig::instance()->debug_flag()),
                     xtag("name", Catch::getResultCapture().getCurrentTestName()));
    }
}

/* end Utest.cpp */
