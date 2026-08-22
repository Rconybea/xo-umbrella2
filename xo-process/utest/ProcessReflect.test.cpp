/* @file ProcessReflect.test.cpp */

#include "xo/process/init_process.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/ppsink/scope.hpp>   /* scope -- was arriving via xo/reflect */
#include <xo/ppsink/scope_macros.hpp>
#include <catch2/catch.hpp>
#include <iostream>

namespace xo {
    using xo::reflect::TypeDescrBase;
    using xo::pp::FlatSink;
    using xo::pp::PpStyle;
    using xo::pp::scope;

    namespace ut {
        static InitEvidence s_init = (InitSubsys<S_process_tag>::require());

        TEST_CASE("process-reflect", "[reflect]") {
            Subsystem::initialize_all();

            char const * c_self = "TEST_CASE:process-reflect";
            constexpr bool c_logging_enabled = true;

            scope log(XO_DEBUG2_(c_logging_enabled, c_self));

            // this ought to work but doesn't (too much output?)...
            //log && log(xo::reflect::reflected_types_printer());

            FlatSink sink(PpStyle::colored(), std::cout.rdbuf());

            xo::reflect::TypeDescrBase::print_reflected_types(sink);
        } /*TEST_CASE(process-reflect)*/

    } /*namespace ut*/
} /*namespace xo*/

/* end ProcessReflect.test.cpp */
