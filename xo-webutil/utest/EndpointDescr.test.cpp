/** @file EndpointDescr.test.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  Cover the rendering surface of xo::web::{Stream,Http}EndpointDescr:
 *
 *  - pretty(PpSink&)        -- the rendering primitive
 *  - Prettifier<X>          -- so sink.pp(x) / xtag("k", x) / tostr(x) work
 *  - display_string()       -- what pybind binds as __repr__
 *  - operator<<             -- the opt-in ostream path in webutil_ostream.hpp
 *
 *  That last one is the reason this file exists at all: webutil_ostream.hpp is
 *  provided for users outside xo, so nothing else in the tree compiles it.  An
 *  uncompiled public printing header is exactly how bpu_iostream.hpp came to
 *  reference a dependency xo-unit had never declared.
 *
 *  NB only the FLAT rendering is asserted here.  Line breaking needs a real
 *  PrettySink, which lives in xo-indentlog2 -- pulling that in would give
 *  xo-webutil a test-only dependency on an arena, to re-test pretty_struct,
 *  which xo-indentlog2/utest/pretty_struct.test.cpp already pins.
 **/

#include "xo/webutil/StreamEndpointDescr.hpp"
#include "xo/webutil/HttpEndpointDescr.hpp"
#include "xo/webutil/webutil_ostream.hpp"
#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/pretty.hpp>
#include <xo/ppsink/tostr_xx.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <string>

namespace xo {
    using xo::web::StreamEndpointDescr;
    using xo::web::HttpEndpointDescr;
    using xo::pp::FlatSink;

    namespace ut {
        namespace {
            /** a stream descriptor with @p uri and no-op subscribe fns **/
            StreamEndpointDescr
            make_stream(const std::string & uri) {
                return StreamEndpointDescr(uri,
                                           xo::web::StreamSubscribeFn{},
                                           xo::web::StreamUnsubscribeFn{});
            }

            /** an http descriptor with @p uri and a no-op endpoint fn **/
            HttpEndpointDescr
            make_http(const std::string & uri) {
                return HttpEndpointDescr(uri, xo::web::HttpEndpointFn{});
            }

            /** render @p x through pretty(), flat **/
            template <typename T>
            std::string
            pretty_str(const T & x) {
                std::stringstream ss;
                FlatSink sink(ss.rdbuf());

                x.pretty(sink);

                return ss.str();
            }

            /** render @p x through the Prettifier, via sink.pp() **/
            template <typename T>
            std::string
            pp_str(const T & x) {
                std::stringstream ss;
                FlatSink sink(ss.rdbuf());

                sink.pp(x);

                return ss.str();
            }

            /** render @p x through operator<< (webutil_ostream.hpp) **/
            template <typename T>
            std::string
            os_str(const T & x) {
                std::stringstream ss;

                ss << x;

                return ss.str();
            }
        } /*namespace*/

        TEST_CASE("stream-endpoint-descr-pretty", "[webutil][endpointdescr]") {
            auto d = make_stream("/stem/${foo}/${bar}");

            REQUIRE(pretty_str(d)
                    == "<StreamEndpointDescr :uri_pattern /stem/${foo}/${bar}>");
        }

        TEST_CASE("http-endpoint-descr-pretty", "[webutil][endpointdescr]") {
            auto d = make_http("/stem/${foo}/${bar}");

            REQUIRE(pretty_str(d)
                    == "<HttpEndpointDescr :uri_pattern /stem/${foo}/${bar}>");
        }

        TEST_CASE("endpoint-descr-quotes-only-when-ambiguous", "[webutil][endpointdescr]") {
            /* unq(): a bare uri pattern is unchanged, but one that would be
             * ambiguous read back gets quoted.  The empty case used to render
             * as "<StreamEndpointDescr :uri_pattern >".
             */
            SECTION("plain uri pattern renders bare") {
                REQUIRE(pretty_str(make_stream("/stem/plain"))
                        == "<StreamEndpointDescr :uri_pattern /stem/plain>");
            }

            SECTION("embedded whitespace forces quoting") {
                REQUIRE(pretty_str(make_stream("/stem/with space"))
                        == "<StreamEndpointDescr :uri_pattern \"/stem/with space\">");
            }

            SECTION("empty uri pattern renders as \"\", not a dangling space") {
                REQUIRE(pretty_str(make_stream(""))
                        == "<StreamEndpointDescr :uri_pattern \"\">");
            }
        }

        TEST_CASE("endpoint-descr-prettifier-is-registered", "[webutil][endpointdescr]") {
            /* the point of Prettifier<> living in the class header: sink.pp(x)
             * works for anyone who has the class, with no extra include.
             */
            auto s = make_stream("/a");
            auto h = make_http("/b");

            REQUIRE(pp_str(s) == pretty_str(s));
            REQUIRE(pp_str(h) == pretty_str(h));
        }

        TEST_CASE("endpoint-descr-display-string", "[webutil][endpointdescr]") {
            /* display_string() is what pybind binds as __repr__ */
            auto s = make_stream("/a/${x}");
            auto h = make_http("/b/${y}");

            REQUIRE(s.display_string() == "<StreamEndpointDescr :uri_pattern /a/${x}>");
            REQUIRE(h.display_string() == "<HttpEndpointDescr :uri_pattern /b/${y}>");
        }

        TEST_CASE("endpoint-descr-ostream-inserter", "[webutil][endpointdescr]") {
            /* webutil_ostream.hpp: the paved road for callers outside xo.
             * Must agree byte-for-byte with the PpSink path -- it is the same
             * pretty() reached through a FlatSink.
             */
            auto s = make_stream("/a/${x}");
            auto h = make_http("/b/${y}");

            REQUIRE(os_str(s) == pretty_str(s));
            REQUIRE(os_str(h) == pretty_str(h));
            REQUIRE(os_str(s) == s.display_string());
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end EndpointDescr.test.cpp */
