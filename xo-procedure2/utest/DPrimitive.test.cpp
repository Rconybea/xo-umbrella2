/** @file DPrimitive.test.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include <xo/procedure2/init_procedure2.hpp>
#include <xo/procedure2/init_primitives.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::scm::Primitives;

    namespace ut {
        static InitEvidence s_init = InitSubsys<S_procedure2_tag>::require();

        TEST_CASE("DPrimitive-init", "[procedure2][DPrimitive]")
        {
            REQUIRE(s_init.evidence());
        }

        TEST_CASE("DPrimitive-n_args", "[procedure2][DPrimitive]")
        {
            // s_mul_gco_gco_pm takes 2 AGCObject args
            REQUIRE(Primitives::s_mul_gco_gco_pm.n_args() == 2);
        }

        TEST_CASE("DPrimitive-is_nary", "[procedure2][DPrimitive]")
        {
            REQUIRE(Primitives::s_mul_gco_gco_pm.is_nary() == false);
        }

    } /*namespace ut*/
} /*namespace xo*/

/* end DPrimitive.test.cpp */
