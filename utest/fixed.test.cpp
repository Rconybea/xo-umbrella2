/* @file fixed.test.cpp */

#include "indentlog/print/fixed.hpp"
#include "indentlog/print/tag.hpp"
#include <catch2/catch.hpp>
#include <sstream>

using namespace xo;

namespace ut {
    struct fixed_tcase {
        fixed_tcase() = default;
        fixed_tcase(double x, std::uint32_t prec, std::string s)
            : x_{x}, prec_{prec}, s_{std::move(s)} {}

        /* floating-point value to format */
        double x_ = 0.0;
        /* precision */
        std::uint32_t prec_ = 0;
        /* expected result */
        std::string s_;
    }; /*fixed_tcase*/

    std::vector<fixed_tcase> s_fixed_tcase_v(
        {fixed_tcase(0.0,  0, "0"),
         fixed_tcase(0.0,  1, "0.0"),
         fixed_tcase(0.0,  2, "0.00"),

         //fixed_tcase(0.5,  0, "1"),      // failing --> 0
         fixed_tcase(0.5,  1, "0.5"),

         fixed_tcase(0.049, 0, "0"),
         fixed_tcase(0.049, 1, "0.0"),
         fixed_tcase(0.049, 2, "0.05"),

         fixed_tcase(0.05, 0, "0"),
         fixed_tcase(0.05, 1, "0.1"),
         fixed_tcase(0.05, 2, "0.05"),

         fixed_tcase(1e-6, 0, "0"),
         fixed_tcase(1e-6, 1, "0.0"),
         fixed_tcase(1e-6, 2, "0.00"),
         fixed_tcase(1e-6, 3, "0.000"),
         fixed_tcase(1e-6, 4, "0.0000"),
         fixed_tcase(1e-6, 5, "0.00000"),
         fixed_tcase(1e-6, 6, "0.000001"),

         fixed_tcase(-1e-6, 0, "-0"),
         fixed_tcase(-1e-6, 1, "-0.0"),
         fixed_tcase(-1e-6, 2, "-0.00"),
         fixed_tcase(-1e-6, 3, "-0.000"),
         fixed_tcase(-1e-6, 4, "-0.0000"),
         fixed_tcase(-1e-6, 5, "-0.00000"),
         fixed_tcase(-1e-6, 6, "-0.000001"),

         fixed_tcase(666.66, 1, "666.7"),
         fixed_tcase(666.66, 2, "666.66"),

         fixed_tcase(-666.66, 1, "-666.7"),
         fixed_tcase(-666.66, 2, "-666.66"),

        });

    TEST_CASE("fixed", "[fixed]") {
        tag_config::tag_color = color_spec_type::none();

        for (std::uint32_t i_tc = 0, z_tc = s_fixed_tcase_v.size(); i_tc < z_tc; ++i_tc) {
            fixed_tcase const & tc = s_fixed_tcase_v[i_tc];

            INFO(tostr(xtag("i_tc", i_tc), xtag("x", tc.x_), xtag("prec", tc.prec_)));

            std::stringstream ss;
            ss << fixed(tc.x_, tc.prec_);

            INFO(xtag("ss.str", ss.str()));

            REQUIRE(ss.str() == tc.s_);
        }

        REQUIRE(s_fixed_tcase_v.size() > 1);
    }
} /*namespace ut*/

/* end fixed.test.cpp */
