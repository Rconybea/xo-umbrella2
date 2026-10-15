/* @file tojsonstr.test.cpp
 *
 * author: Roland Conybeare, Oct 2026
 *
 * xo::pp::tojsonstr -- tostr() through a json-styled sink: escapes come out
 * in JSON's vocabulary (\u00NN), not display's (\xNN).
 * See .xo-backlog/xo-printjson/issues/04
 */

#include "print/tostr.hpp"
#include <xo/ppsink/quoted.hpp>
#include <catch2/catch.hpp>
#include <string>
#include <string_view>

namespace xo {
    namespace ut {
        using xo::pp::tojsonstr;
        using xo::pp::tostr;
        using xo::pp::quot;

        TEST_CASE("tojsonstr-escapes-for-json", "[tojsonstr]") {
            std::string_view s("a\0b", 3);

            REQUIRE(tojsonstr(quot(s)) == "\"a\\u0000b\"");
            REQUIRE(tojsonstr(quot("\x1b\x7f")) == "\"\\u001b\\u007f\"");
            /* short forms: the same in both vocabularies */
            REQUIRE(tojsonstr(quot("q\"\n")) == "\"q\\\"\\n\"");
        }

        TEST_CASE("tostr-still-escapes-for-display", "[tojsonstr]") {
            std::string_view s("a\0b", 3);

            REQUIRE(tostr(quot(s)) == "\"a\\x00b\"");
        }

        /* like tostr: several arguments, concatenated */
        TEST_CASE("tojsonstr-concatenates", "[tojsonstr]") {
            REQUIRE(tojsonstr("{", quot("k\x01"), ": ", 1, "}") == "{\"k\\u0001\": 1}");
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end tojsonstr.test.cpp */
