/** @file Utest.hpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#pragma once

#include <xo/ppsink/scope.hpp>

namespace xo {

    /** RAII logging for catch2 unit tests
     *
     *  Use:
     *    TEST_CASE(name, tags, ..)
     *    {
     *       auto log = Utest::ut_scope();
     *
     *       ...
     *       log && log(xo::pp::xtag("foo", ...));
     *    }
     *
     *  Honors:
     *    UtestConfig::instance()->debug_flag_
     **/
    struct Utest {
        /** the (ppsink) scope-logger type that ut_scope() hands out **/
        using scope = xo::pp::scope;

        /** Toplevel logging scope for unit tests.
         *  Integrates with UtestConfig
         **/
        static scope ut_scope();
    };

} /*namespace xo*/

/* end Utest.hpp */
