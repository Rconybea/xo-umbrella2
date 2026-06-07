/** @file Utest.hpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#pragma once

#include <xo/indentlog/scope.hpp>

namespace xo {

    /** RAII logging for catch2 unit tests
     *
     *  Use:
     *    TEST_CASE(name, tags, ..)
     *    {
     *       scope log = Utest::ut_scope();
     *
     *       ...
     *       log && log(xtag("foo", ...));
     *    }
     *
     *  Honors:
     *    UtestConfig::instance()->debug_flag_
     **/
    struct Utest {
        /** Toplevel logging scope for unit tests.
         *  Integrates with UtestConfig
         **/
        static scope ut_scope();
    };

} /*namespace xo*/

/* end Utest.hpp */
