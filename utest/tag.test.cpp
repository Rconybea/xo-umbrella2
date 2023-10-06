/* @file tag.test.cpp */

#include "xo/indentlog/print/tag.hpp"
#include "xo/indentlog/print/vector.hpp"
#include "xo/indentlog/print/concat.hpp"
#include <catch2/catch.hpp>
#include <sstream>

using namespace xo;

namespace ut {
    TEST_CASE("tag", "[tag]") {
        tag_config::tag_color = color_spec_type::none();

        {
            std::stringstream ss;
            ss << tag("foo", "hello,world!");

            REQUIRE(ss.str() == ":foo hello,world!");
        }

        {
            std::stringstream ss;
            ss << tag("foo", "hello, world!");

            REQUIRE(ss.str() == ":foo \"hello, world!\"");
        }

        {
            std::stringstream ss;
            std::vector<int> v = {1, 2, 3};
            ss << tag("foo", v);

            REQUIRE(ss.str() == ":foo \"[1 2 3]\"");
        }

        {
            std::stringstream ss;
            ss << tag("foo", concat("farenheit", 451));

            REQUIRE(ss.str() == ":foo farenheit451");
        }

        {
            std::stringstream ss;
            ss << tag("foo", "hello") << xtag("bar", "there");

            REQUIRE(ss.str() == ":foo hello :bar there");
        }

        tag_config::tag_color = color_spec_type::blue();

        {
            std::stringstream ss;
            ss << tag("foo", "hello,world!");

            /*                   color on       color off
             *                   <--------->    <----->
             *
             * see [indentlog/print/color.hpp] for escape sequences
             */
            REQUIRE(ss.str() == "\033[31;34m:foo\033[0m hello,world!");
        }
    } /*TEST_CASE(tag)*/
} /*namespace ut*/

/* end tag.test.cpp */
