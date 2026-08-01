/** @file Utest.cpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#include "Utest.hpp"
#include "UtestConfig.hpp"
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::pp::scope;
    using xo::pp::xtag;

    scope
    Utest::ut_scope() {
        return scope(XO_DEBUG_(UtestConfig::instance()->debug_flag()),
                     xtag("name", Catch::getResultCapture().getCurrentTestName()));
    }
}

/* end Utest.cpp */
