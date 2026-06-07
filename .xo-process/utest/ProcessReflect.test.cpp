/* @file ProcessReflect.test.cpp */

#include "xo/process/init_process.hpp"
#include "xo/reflect/Reflect.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::reflect::TypeDescrBase;

    namespace ut {
        static InitEvidence s_init = (InitSubsys<S_process_tag>::require());

        TEST_CASE("process-reflect", "[reflect]") {
            Subsystem::initialize_all();

            char const * c_self = "TEST_CASE:process-reflect";
            constexpr bool c_logging_enabled = true;

            scope log(XO_DEBUG2(c_logging_enabled, c_self));

            // this ought to work but doesn't (too much output?)...
            //log && log(xo::reflect::reflected_types_printer());

            xo::reflect::TypeDescrBase::print_reflected_types(std::cout);
        } /*TEST_CASE(process-reflect)*/

    } /*namespace ut*/
} /*namespace xo*/

/* end ProcessReflect.test.cpp */
