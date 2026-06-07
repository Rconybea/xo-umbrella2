/* @file code_location.test.cpp */

#include "xo/indentlog/print/code_location.hpp"
#include "xo/indentlog/print/color.hpp"
#include "xo/indentlog/print/tag.hpp"
#include <vector>
#include <catch2/catch.hpp>

using namespace xo;

namespace ut {
    struct code_location_tcase {
        code_location_tcase() = default;
        code_location_tcase(std::string_view file, std::uint32_t line, color_spec_type color, std::string_view output)
            : file_{file}, line_{line}, color_{color}, output_{output} {}

        /* target time value to test */
        std::string_view file_;
        std::uint32_t line_;
        color_spec_type color_;
        std::string_view output_;
    }; /*code_location_tcase*/

    std::vector<code_location_tcase> s_code_location_tcase_v(
        {
            code_location_tcase("/foo/bar", 123, color_spec_type::none(), "[bar:123]"),
            code_location_tcase("/foo/bar", 123, color_spec_type::blue(), "[\033[31;34mbar\033[0m:123]"),
            code_location_tcase("/foo/bar", 123, color_spec_type::xterm(196), "[\033[38;5;196mbar\033[0m:123]"),
            code_location_tcase("/foo/bar", 123, color_spec_type::rgb(255, 127, 63), "[\033[38;2;255;127;63mbar\033[0m:123]"),
        });

    TEST_CASE("code_location", "[code_location]") {
        for (std::uint32_t i_tc = 0, z_tc = s_code_location_tcase_v.size(); i_tc < z_tc; ++i_tc) {
            code_location_tcase const & tc = s_code_location_tcase_v[i_tc];

            INFO(tostr(xtag("i_tc", i_tc), xtag("file", tc.file_), xtag("line", tc.line_), xtag("color", tc.color_)));
            INFO(xtag("tc.output", tc.output_));

            std::stringstream ss;
            ss << code_location(tc.file_, tc.line_, tc.color_);

            REQUIRE(ss.str() == tc.output_);
        }
    } /*TEST_CASE(code_location)*/
} /*namespace ut*/

/* end code_location.test.cpp */
