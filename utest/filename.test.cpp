/* @file filename.test.cpp */

#include "xo/indentlog/print/filename.hpp"
#include "xo/indentlog/print/tag.hpp"
#include <vector>
#include <catch2/catch.hpp>

using namespace xo;

namespace ut {
    struct filename_tcase {
        filename_tcase() = default;
        filename_tcase(std::string_view path, std::string_view basename)
            : path_{path}, basename_{basename} {}

        /* target time value to test */
        std::string_view path_;
        std::string_view basename_;
    }; /*filename_tcase*/

    std::vector<filename_tcase> s_filename_tcase_v(
        {
            filename_tcase("foo", "foo"),
            filename_tcase("/foo", "foo"),
            filename_tcase("/foo/bar", "bar"),
        });

    TEST_CASE("filename", "[filename]") {
        for (std::uint32_t i_tc = 0, z_tc = s_filename_tcase_v.size(); i_tc < z_tc; ++i_tc) {
            filename_tcase const & tc = s_filename_tcase_v[i_tc];

            INFO(tostr(xtag("i_tc", i_tc), xtag("path", tc.path_)));
            INFO(xtag("tc.basename", tc.basename_));

            std::stringstream ss;
            ss << basename(tc.path_);

            REQUIRE(ss.str() == tc.basename_);
        }
    } /*TEST_CASE(filename)*/
} /*namespace ut*/

/* end filename.test.cpp */
