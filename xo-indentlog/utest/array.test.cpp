/* @file array.test.cpp */

#include "xo/indentlog/print/array.hpp"  /* overload operator<< for std::array */
#include "xo/indentlog/print/tag.hpp"
#include <catch2/catch.hpp>
#include <sstream>

using namespace xo;

namespace ut {
    TEST_CASE("array", "[array]") {
        tag_config::tag_color = color_spec_type::none();

        {
            std::array<int, 0> x = {};
            std::stringstream ss;
            ss << x;

            REQUIRE(ss.str() == "[]");
        }

        {
            std::array<int, 1> x = {1};
            std::stringstream ss;
            ss << x;

            REQUIRE(ss.str() == "[1]");
        }

        {
            std::array<int, 2> x = {1, 2};
            std::stringstream ss;
            ss << x;

            REQUIRE(ss.str() == "[1 2]");
        }
    }
} /*namespace ut*/

/* end array.test.cpp */
