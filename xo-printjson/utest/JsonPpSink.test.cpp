/** @file JsonPpSink.test.cpp
 *
 *  JsonPpSink: json written through labelled entry points -- separators
 *  inserted, keys and values alternate, layout groups for a PrettySink.
 *  See .xo-backlog/xo-printjson/issues/11
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "xo/printjson/JsonPpSink.hpp"
#include <xo/indentlog2/print/PrettySink.hpp>
#include <xo/indentlog2/print/TempFlatSink.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <catch2/catch.hpp>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

namespace xo {
    namespace ut {
        using xo::json::JsonPpSink;
        using xo::pp::PpConfig;
        using xo::pp::PpStyle;
        using xo::pp::PrettySink;
        using xo::pp::TempFlatSink;

        TEST_CASE("json-pp-sink-scalars-in-an-object", "[JsonPpSink]") {
            TempFlatSink out(PpStyle::json());
            JsonPpSink json(&out.sink());

            json.open_object()
                .key("s").string("x\"y")
                .key("i").number(-3)
                .key("u").number(std::uint64_t{7})
                .key("d").number(1.5)
                .key("b").boolean(true)
                .key("n").null()
                .close_object();

            REQUIRE(out.output() == R"({"s": "x\"y", "i": -3, "u": 7, "d": 1.5, "b": true, "n": null})");
        }

        TEST_CASE("json-pp-sink-nested-and-empty", "[JsonPpSink]") {
            TempFlatSink out(PpStyle::json());
            JsonPpSink json(&out.sink());

            json.open_list()
                .number(1)
                .open_object().close_object()
                .open_list().close_list()
                .open_object().key("v").open_list().number(2).number(3).close_list().close_object()
                .close_list();

            REQUIRE(out.output() == R"([1, {}, [], {"v": [2, 3]}])");
        }

        /* a list's separator spacing: "," alone when sep_spaces is 0 */
        TEST_CASE("json-pp-sink-compact-list", "[JsonPpSink]") {
            TempFlatSink out(PpStyle::json());
            JsonPpSink json(&out.sink());

            json.open_list()
                .open_list(0).number(1).number(2).close_list()
                .open_list(0).number(3).number(4).close_list()
                .close_list();

            REQUIRE(out.output() == "[[1,2], [3,4]]");
        }

        /* json object keys are strings: a value written where a key is
         * expected becomes that key
         */
        TEST_CASE("json-pp-sink-value-in-key-position", "[JsonPpSink]") {
            TempFlatSink out(PpStyle::json());
            JsonPpSink json(&out.sink());

            json.open_object()
                .number(3).string("three")
                .boolean(false).number(0)
                .string("k").null()
                .close_object();

            REQUIRE(out.output() == R"({"3": "three", "false": 0, "k": null})");
        }

        /* a value that is not json -- a diagnostic, or the NaN extension --
         * still takes its place in the list
         */
        TEST_CASE("json-pp-sink-raw-value", "[JsonPpSink]") {
            TempFlatSink out(PpStyle::json());
            JsonPpSink json(&out.sink());

            json.open_list().number(1).raw("NaN").close_list();

            REQUIRE(out.output() == "[1, NaN]");
        }

        /* the bottom of the stack takes any number of top-level values, with
         * nothing between them -- the caller's business
         */
        TEST_CASE("json-pp-sink-several-top-level-values", "[JsonPpSink]") {
            TempFlatSink out(PpStyle::json());
            JsonPpSink json(&out.sink());

            json.number(1);
            out.sink().put(" ");
            json.string("a");

            REQUIRE(out.output() == R"(1 "a")");
        }

        TEST_CASE("json-pp-sink-misuse-throws", "[JsonPpSink]") {
            TempFlatSink out(PpStyle::json());

            {
                JsonPpSink json(&out.sink());
                json.open_list();
                REQUIRE_THROWS_AS(json.key("k"), std::logic_error);
            }
            {
                JsonPpSink json(&out.sink());
                json.open_object().key("k");
                REQUIRE_THROWS_AS(json.close_object(), std::logic_error);   /* dangling key */
            }
            {
                JsonPpSink json(&out.sink());
                json.open_object();
                REQUIRE_THROWS_AS(json.open_list(), std::logic_error);      /* a list as a key */
                REQUIRE_THROWS_AS(json.raw("x"), std::logic_error);
            }
            {
                JsonPpSink json(&out.sink());
                json.open_object();
                REQUIRE_THROWS_AS(json.close_list(), std::logic_error);     /* wrong bracket */
            }
            {
                JsonPpSink json(&out.sink());
                REQUIRE_THROWS_AS(json.close_object(), std::logic_error);   /* nothing open */
            }
        }

        /* a display-escaping sink would write \xNN into the document */
        TEST_CASE("json-pp-sink-refuses-display-sink", "[JsonPpSink]") {
            TempFlatSink out(PpStyle::plain());

            REQUIRE_THROWS_AS(JsonPpSink(&out.sink()), std::runtime_error);
        }

        /* layout: a group that does not fit breaks, one item per line */
        TEST_CASE("json-pp-sink-pretty", "[JsonPpSink]") {
            PrettySink pp(PpConfig::scratch_plain(20).with_style(PpStyle::json()),
                          nullptr /*out*/);
            JsonPpSink json(&pp);

            json.open_object()
                .key("alpha").number(1)
                .key("beta").open_list().number(2).number(3).close_list()
                .close_object();

            REQUIRE(std::string(pp.output())
                    == "{\n  \"alpha\": 1,\n  \"beta\": [2, 3]\n}");
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end JsonPpSink.test.cpp */
