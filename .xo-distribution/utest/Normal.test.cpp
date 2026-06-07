/* @file Normal.test.cpp */

#include "xo/distribution/Normal.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::distribution::Normal;

    namespace ut {
        TEST_CASE("normal", "[distribution]") {
            auto n01 = Normal::unit();

            CHECK(n01->cdf(-3.0) == Approx(0.001349898).margin(1e-9));
            CHECK(n01->cdf(-2.0) == Approx(0.0227501319).margin(1e-9));
            CHECK(n01->cdf(-1.0) == Approx(0.1586552539).margin(1e-9));
            CHECK(n01->cdf(0.0) == 0.5);
            CHECK(n01->cdf(1.0) == 1.0 - n01->cdf(-1.0));
            CHECK(n01->cdf(2.0) == 1.0 - n01->cdf(-2.0));
            CHECK(n01->cdf(3.0) == 1.0 - n01->cdf(-3.0));
        } /*TEST_CASE(normal)*/
    } /*namespace ut*/
} /*namespace xo*/

/* end Normal.test.cpp */
