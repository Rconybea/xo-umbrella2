/* @file Uniform.test.cpp */

#include "xo/distribution/Uniform.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::distribution::Uniform;

    namespace ut {
        TEST_CASE("uniform", "[distribution]") {
            auto u = Uniform::unit();

            CHECK(u->cdf(-3.0) == 0.0);
            CHECK(u->cdf(-2.0) == 0.0);
            CHECK(u->cdf(-1.0) == 0.0);
            CHECK(u->cdf(0.0) == 0.0);
            CHECK(u->cdf(0.05) == 0.05);
            CHECK(u->cdf(0.5) == 0.5);
            CHECK(u->cdf(0.95) == 0.95);
            CHECK(u->cdf(1.0) == 1.0);
            CHECK(u->cdf(2.0) == 1.0);
            CHECK(u->cdf(3.0) == 1.0);
        } /*TEST_CASE(uniform)*/
    } /*namespace ut*/
} /*namespace xo*/

/* end Uniform.test.cpp */
