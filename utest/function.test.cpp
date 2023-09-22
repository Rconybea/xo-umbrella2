/* @file function.test.cpp */

#include "indentlog/print/function.hpp"
#include "indentlog/print/tag.hpp"
#include <catch2/catch.hpp>
#include <sstream>

using namespace xo;

namespace ut {
    struct function_tcase {
        function_tcase() = default;
        function_tcase(function_style style, color_spec_type spec, std::string_view pretty, std::string_view output)
            : style_{style}, spec_{spec}, pretty_{pretty}, output_{output} {}

        /* function style: literal|pretty|streamlined|simple*/
        function_style style_;
        /* color spec for output */
        color_spec_type spec_;
        /* function signature (as per __PRETTY_FUNCTION__) */
        std::string_view pretty_;
        /* output text */
        std::string_view output_;
    }; /*function_tcase*/

    std::vector<function_tcase> s_function_tcase_v(
        {
            function_tcase(function_style::pretty,      color_spec_type::none(), "void foo() const", "[void foo() const]"),
            function_tcase(function_style::streamlined, color_spec_type::none(), "void foo() const", "foo"),
            function_tcase(function_style::simple,      color_spec_type::none(), "void foo() const", "foo"),

            function_tcase(function_style::pretty,      color_spec_type::none(), "void xo::class::foo() const", "[void xo::class::foo() const]"),
            function_tcase(function_style::streamlined, color_spec_type::none(), "void xo::class::foo() const", "class::foo"),
            function_tcase(function_style::simple,      color_spec_type::none(), "void xo::class::foo() const", "foo"),

            function_tcase(function_style::pretty,      color_spec_type::blue(), "void xo::class::foo() const", "[\033[31;34mvoid xo::class::foo() const\033[0m]"),
        });

    TEST_CASE("function", "[function]") {
        tag_config::tag_color = color_spec_type::none();

        for (std::uint32_t i_tc = 0, z_tc = s_function_tcase_v.size(); i_tc < z_tc; ++i_tc) {
            function_tcase const & tc = s_function_tcase_v[i_tc];

            INFO(tostr(xtag("i_tc", i_tc), xtag("style", tc.style_), xtag("spec", tc.spec_), xtag("pretty", tc.pretty_)));

            std::stringstream ss;
            ss << function_name(tc.style_, tc.spec_, tc.pretty_);

            INFO(xtag("ss.str", ss.str()));

            REQUIRE(ss.str() == tc.output_);
        }

        REQUIRE(s_function_tcase_v.size() > 1);
    }
} /*namespace ut*/

/* end function.test.cpp */
