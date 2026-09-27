/** @file Webserver.test.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  The Webserver's endpoint registration API, on a server that is made but
 *  never started: no socket, no service thread.  What needs a running server
 *  -- ending live subscriptions on unregister -- is covered at the router
 *  level (WsSessionRouter.test.cpp, [removal]).
 *
 *  Expectations are OBSERVED, never predicted.
 **/

#include "xo/websock/Webserver.hpp"
#include <xo/printjson/PrintJsonSingleton.hpp>
#include <catch2/catch.hpp>
#include <json/json.h>
#include <stdexcept>
#include <string>

namespace xo {
    using xo::web::Webserver;
    using xo::web::WebserverConfig;
    using xo::web::HttpEndpointDescr;
    using xo::web::StreamEndpointDescr;
    using xo::web::WebsocketSink;
    using xo::web::Alist;
    using xo::web::Runstate;
    using xo::json::PrintJsonSingleton;
    using xo::fn::CallbackId;

    namespace ut {
        namespace {
            rp<Webserver> make_idle_server() {
                /* port never bound: start_webserver() is not called */
                return Webserver::make(WebserverConfig(), PrintJsonSingleton::instance());
            }

            HttpEndpointDescr http_descr(std::string pattern) {
                return HttpEndpointDescr(std::move(pattern),
                                         [](std::string const &, Alist const &, std::ostream *) {});
            }

            StreamEndpointDescr stream_descr(std::string pattern) {
                return StreamEndpointDescr(std::move(pattern),
                                           [](rp<WebsocketSink> const &) { return CallbackId(1); },
                                           [](CallbackId) {});
            }
        }

        TEST_CASE("webserver-unregister-stream-endpoint", "[websock][Webserver]")
        {
            rp<Webserver> websrv = make_idle_server();

            REQUIRE(websrv->state() == Runstate::stopped);

            websrv->register_stream_endpoint(stream_descr("/fw/${id}"));

            /* a duplicate stem is rejected until unregistered */
            REQUIRE_THROWS_AS(websrv->register_stream_endpoint(stream_descr("/fw/${x}")),
                              std::runtime_error);

            /* exact pattern only */
            REQUIRE(!websrv->unregister_stream_endpoint("/fw/${x}"));
            REQUIRE(websrv->unregister_stream_endpoint("/fw/${id}"));
            REQUIRE(!websrv->unregister_stream_endpoint("/fw/${id}"));

            /* the stem is free again */
            REQUIRE_NOTHROW(websrv->register_stream_endpoint(stream_descr("/fw/${x}")));
        }

        TEST_CASE("webserver-unregister-http-endpoint", "[websock][Webserver]")
        {
            rp<Webserver> websrv = make_idle_server();

            websrv->register_http_endpoint(http_descr("/status"));
            websrv->register_stream_endpoint(stream_descr("/status"));

            /* http and stream are separate: removing one leaves the other */
            REQUIRE(websrv->unregister_http_endpoint("/status"));
            REQUIRE(!websrv->unregister_http_endpoint("/status"));
            REQUIRE(websrv->unregister_stream_endpoint("/status"));
        }

        TEST_CASE("webserver-lists-its-endpoints", "[websock][Webserver]")
        {
            rp<Webserver> websrv = make_idle_server();

            REQUIRE(websrv->endpoints().empty());

            websrv->register_stream_endpoint(stream_descr("/fw/${id}"));
            websrv->register_http_endpoint(http_descr("/status"));

            auto v = websrv->endpoints();

            REQUIRE(v.size() == 2);
            REQUIRE(v[0].kind_ == xo::web::EndpointKind::http);
            REQUIRE(v[0].uri_pattern_ == "/status");
            REQUIRE(v[1].kind_ == xo::web::EndpointKind::stream);
            REQUIRE(v[1].uri_pattern_ == "/fw/${id}");

            REQUIRE(websrv->unregister_stream_endpoint("/fw/${id}"));
            REQUIRE(websrv->endpoints().size() == 1);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end Webserver.test.cpp */
