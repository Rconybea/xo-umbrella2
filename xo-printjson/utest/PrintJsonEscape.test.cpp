/** @file PrintJsonEscape.test.cpp
 *
 *  strings -- values and keys -- escape for JSON, not for display:
 *  \u00NN, never \xNN (.xo-backlog/xo-printjson/issues/04)
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "xo/printjson/PrintJson.hpp"
#include <xo/reflect/StructReflector.hpp>
#include <catch2/catch.hpp>
#include <map>
#include <sstream>
#include <string>
#include <string_view>

namespace xo {
    using xo::json::PrintJson;
    using xo::reflect::StructReflector;

    namespace ut {
        namespace {
            struct Keyed {
                std::map<std::string, int> by_name_;
            };

            void reflect_types() {
                static bool s_once = []() {
                    StructReflector<Keyed> sr;
                    sr.reflect_member("by_name", &Keyed::by_name_);
                    return true;
                }();
                (void)s_once;
            }

            template <typename T>
            std::string print(T const & x) {
                PrintJson pjson;
                std::stringstream ss;
                pjson.print(x, &ss);
                return ss.str();
            }

            bool contains(std::string const & s, std::string const & pat) {
                return s.find(pat) != std::string::npos;
            }
        } /*namespace*/

        TEST_CASE("json-string-control-chars", "[printjson][escape]") {
            /* an embedded NUL: JSON has no \x00 -- a strict parser rejects it */
            REQUIRE(print(std::string("a\0b", 3)) == "\"a\\u0000b\"");
            REQUIRE(print(std::string("\x1b[31m")) == "\"\\u001b[31m\"");
            /* DEL: valid raw in JSON, but invisible; escaped like the rest */
            REQUIRE(print(std::string("\x7f")) == "\"\\u007f\"");
        }

        TEST_CASE("json-string-short-forms", "[printjson][escape]") {
            REQUIRE(print(std::string("q\"b\\")) == "\"q\\\"b\\\\\"");
            REQUIRE(print(std::string("a\nb\tc")) == "\"a\\nb\\tc\"");
            /* UTF-8 passes through */
            REQUIRE(print(std::string("caf\xc3\xa9")) == "\"caf\xc3\xa9\"");
        }

        /* the four string printers share the path */
        TEST_CASE("json-string-printers", "[printjson][escape]") {
            char const * cs = "a\x01";
            std::string_view sv("a\x01");

            REQUIRE(print(cs) == "\"a\\u0001\"");
            REQUIRE(print(sv) == "\"a\\u0001\"");
        }

        /* map keys are user data, and print through a different call site
         * than values
         */
        TEST_CASE("json-map-key-escapes", "[printjson][escape]") {
            reflect_types();

            Keyed k;
            k.by_name_["k\x01\"q"] = 1;

            std::string s = print(k);

            INFO(s);
            REQUIRE(contains(s, "\"k\\u0001\\\"q\": 1"));
            REQUIRE(!contains(s, "\\x"));
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end PrintJsonEscape.test.cpp */
