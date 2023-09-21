/* @file vector.test.cpp */

#include "indentlog/print/vector.hpp"  /* overload operator<< for std::vector */
#include "indentlog/print/tag.hpp"
#include <catch2/catch.hpp>
#include <sstream>

using namespace xo;

namespace ut {
    struct vector_tcase {
        vector_tcase() = default;
        vector_tcase(std::vector<int> const & x, std::string s)
            : x_{x}, s_{std::move(s)} {}

        /* vector to print */
        std::vector<int> x_;
        /* expected result */
        std::string s_;
    }; /*vector_tcase*/

    std::vector<vector_tcase> s_vector_tcase_v(
        {vector_tcase({}, "[]"),
         vector_tcase({1}, "[1]"),
         vector_tcase({1, 2}, "[1 2]"),
         vector_tcase({10, 20, 30}, "[10 20 30]"),

        });

    TEST_CASE("vector", "[vector]") {
        tag_config::tag_color = color_spec_type::none();

        for (std::uint32_t i_tc = 0, z_tc = s_vector_tcase_v.size(); i_tc < z_tc; ++i_tc) {
            vector_tcase const & tc = s_vector_tcase_v[i_tc];

            INFO(tostr(xtag("i_tc", i_tc), xtag("x", tc.x_)));

            std::stringstream ss;
            ss << tc.x_;

            INFO(xtag("ss.str", ss.str()));

            REQUIRE(ss.str() == tc.s_);
        }

        REQUIRE(s_vector_tcase_v.size() > 1);
    }
} /*namespace ut*/

/* end vector.test.cpp */
