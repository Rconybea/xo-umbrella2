/** @file PrintJsonSink.test.cpp
 *
 *  json written into a PpSink: JsonPrintState takes a sink, and refuses one
 *  that does not escape for json (.xo-backlog/xo-printjson/issues/01,
 *  "Decision")
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "xo/printjson/PrintJson.hpp"
#include "xo/printjson/JsonPrintState.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/StructReflector.hpp>
#include <xo/indentlog2/print/PrettySink.hpp>
#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <catch2/catch.hpp>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace xo {
    using xo::json::JsonPrintState;
    using xo::json::PrintJson;
    using xo::pp::FlatSink;
    using xo::pp::PpConfig;
    using xo::pp::PpStyle;
    using xo::pp::PrettySink;
    using xo::reflect::Reflect;
    using xo::reflect::StructReflector;

    namespace ut {
        namespace {
            struct Sample {
                int n_ = 7;
                double x_ = 1.5;
                std::string s_ = "a\x01\"b";
                std::vector<int> v_ = {1, 2, 3};
                std::map<std::string, int> m_ = {{"k", 1}};
            };

            void reflect_types() {
                static bool s_once = []() {
                    StructReflector<Sample> sr;
                    sr.reflect_member("n", &Sample::n_);
                    sr.reflect_member("x", &Sample::x_);
                    sr.reflect_member("s", &Sample::s_);
                    sr.reflect_member("v", &Sample::v_);
                    sr.reflect_member("m", &Sample::m_);
                    return true;
                }();
                (void)s_once;
            }

            /** through the std::ostream entry point **/
            std::string print_to_ostream(PrintJson const & pjson, Sample const & x) {
                std::stringstream ss;
                pjson.print(x, &ss);
                return ss.str();
            }
        } /*namespace*/

        /* a display-escaping sink would write \xNN into the document */
        TEST_CASE("json-print-state-refuses-display-sink", "[printjson][sink]") {
            PrintJson pjson;
            std::stringstream ss;
            FlatSink sink(PpStyle::plain(), ss.rdbuf());

            REQUIRE_THROWS_AS(JsonPrintState(&pjson, &sink), std::runtime_error);
        }

        /* colour is the caller's choice: json for a human to read (RC) */
        TEST_CASE("json-print-state-accepts-colored-json-sink", "[printjson][sink]") {
            PrintJson pjson;
            std::stringstream ss;
            PpStyle style = PpStyle::json();
            style.color_enabled = true;
            FlatSink sink(style, ss.rdbuf());

            REQUIRE_NOTHROW(JsonPrintState(&pjson, &sink));
        }

        /* the sink entry point and the ostream entry point agree, byte for byte */
        TEST_CASE("print-tp-into-sink-matches-ostream", "[printjson][sink]") {
            reflect_types();

            PrintJson pjson;
            Sample x;

            std::stringstream ss;
            FlatSink sink(PpStyle::json(), ss.rdbuf());
            pjson.print_tp(Reflect::make_tp(&x), &sink);

            INFO(ss.str());
            REQUIRE(ss.str() == print_to_ostream(pjson, x));
        }

        /* everything a printer writes -- p_os() included -- reaches the sink,
         * in order.  A PrettySink buffers tokens, so a write that went around
         * it, straight to an underlying streambuf, would be missing or out of
         * order here.  No printer emits structure yet, so pretty == flat
         */
        TEST_CASE("print-tp-into-pretty-sink-routes-through-sink", "[printjson][sink]") {
            reflect_types();

            PrintJson pjson;
            Sample x;

            PrettySink pp(PpConfig::scratch_plain(1000).with_style(PpStyle::json()),
                          nullptr /*out*/);
            pjson.print_tp(Reflect::make_tp(&x), &pp);

            REQUIRE(std::string(pp.output()) == print_to_ostream(pjson, x));
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end PrintJsonSink.test.cpp */
