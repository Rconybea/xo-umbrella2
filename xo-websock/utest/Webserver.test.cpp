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

#include "WebsockUtestAppcx.hpp"
#include "xo/websock/Webserver.hpp"
#include "xo/websock/DynamicEndpoint.hpp"
#include <xo/printjson/PrintJsonSingleton.hpp>
#include <catch2/catch.hpp>
#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/StructReflector.hpp>
#include <xo/reflectutil/type_name.hpp>
#include <json/json.h>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <string>

namespace xo {
    using xo::web::Webserver;
    using xo::web::WebserverConfig;
    using xo::web::HttpEndpointDescr;
    using xo::web::StreamEndpointDescr;
    using xo::web::WebsocketSink;
    using xo::web::HttpRequest;
    using xo::web::HttpResponse;
    using xo::web::Runstate;
    using xo::json::PrintJsonSingleton;
    using xo::fn::CallbackId;
    using xo::reflect::Reflect;
    using xo::reflect::StructReflector;

    namespace ut {
        namespace {
            /** what the introspect example sends: a reflected struct holding
             *  a Webserver*.  PrintJson follows the pointer to the Webserver
             *  printer (xo/websock/websock_json.hpp)
             **/
            struct HoldsServer {
                static void reflect_self() {
                    StructReflector<HoldsServer> sr;

                    if (sr.is_incomplete())
                        REFLECT_MEMBER(sr, server);
                }

                Webserver * server_ = nullptr;
            };

            Json::Value parse_json(std::string const & text) {
                Json::Value root;
                JSONCPP_STRING err;
                std::unique_ptr<Json::CharReader> rd(Json::CharReaderBuilder().newCharReader());

                bool ok = rd->parse(text.data(), text.data() + text.size(), &root, &err);

                INFO("text: " << text << " err: " << err);
                REQUIRE(ok);

                return root;
            }
            rp<Webserver> make_idle_server() {
                /* port never bound: start_webserver() is not called */
                return Webserver::make(WebsockUtestAppcx::appcx().cx<S_websock_tag>(), WebserverConfig());
            }

            HttpEndpointDescr http_descr(std::string pattern) {
                return HttpEndpointDescr(std::move(pattern),
                                         [](HttpRequest const &) { return HttpResponse::json("{}"); });
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

        TEST_CASE("webserver-visits-its-endpoints", "[websock][Webserver]")
        {
            rp<Webserver> websrv = make_idle_server();

            std::vector<std::string> v;
            auto visit = [&] {
                v.clear();
                websrv->visit_endpoints([&v](xo::web::DynamicEndpoint const & ep) {
                        v.push_back(ep.uri_pattern());
                    });
            };

            visit();
            REQUIRE(v.empty());

            websrv->register_stream_endpoint(stream_descr("/fw/${id}"));
            websrv->register_http_endpoint(http_descr("/status"));

            visit();
            REQUIRE(v == std::vector<std::string>{"/status", "/fw/${id}"});

            REQUIRE(websrv->unregister_stream_endpoint("/fw/${id}"));
            visit();
            REQUIRE(v == std::vector<std::string>{"/status"});
        }

        TEST_CASE("websock-types-are-reflected", "[websock][reflect]")
        {
            /* the context describes xo-websock's types to xo-reflect
             * (websock_reflect_types), including ones private to a .cpp
             */
            rp<Webserver> websrv = make_idle_server();

            for (char const * name : {"xo::web::Webserver",
                                      "xo::web::WebserverImpl",
                                      "xo::web::WebserverConfig",
                                      "xo::web::WebsocketSessionRecd",
                                      "xo::web::WsSessionSender<xo::web::WebserverImpl>",
                                      "xo::web::WsSessionTable<xo::web::WebsocketSessionRecd>",
                                      "xo::web::WebsocketSink",
                                      "xo::web::WebsocketSinkImpl",
                                      "xo::web::WsSessionRouter",
                                      "xo::web::WsSessionRouter::Subscription",
                                      "xo::web::DynamicEndpoint",
                                      "xo::web::UrlRouter"})
            {
                INFO(name);
                auto td = xo::reflect::TypeDescrBase::lookup_by_name(name);

                REQUIRE(td);
                REQUIRE(td->is_struct());
                REQUIRE(td->complete_flag());
            }

            /* SelfTaggingDisplayable: from a Webserver*, the actual type */
            Webserver * w = websrv.get();
            auto most = Reflect::require<Webserver>()->most_derived_self_tp(w);

            REQUIRE(most.td()->canonical_name() == "xo::web::WebserverImpl");
            REQUIRE(most.address() == static_cast<void *>(w));
        }

        namespace {
            /* a stream receiver; SelfTagging, not reflected in full */
            class NamedReceiver : public xo::web::StreamReceiver {
            public:
                xo::reflect::TaggedRcptr self_tp() override { return Reflect::make_rctp(this); }
                void receive(rp<WebsocketSink> const &, Json::Value const &) override {}
            };
        }

        TEST_CASE("webserver-json-names-each-receiver", "[websock][Webserver][json]")
        {
            /* an endpoint's receiver printed in full: named by its most-
             * derived type (StreamReceiver is SelfTagging), with the id the
             * receiver_ member's ref uses
             */
            HoldsServer::reflect_self();

            rp<Webserver> websrv = make_idle_server();
            rp<NamedReceiver> recv(new NamedReceiver());
            websrv->register_http_endpoint(http_descr("/status"));
            websrv->register_stream_endpoint(
                StreamEndpointDescr("/fw/${id}",
                                    [](rp<WebsocketSink> const &) { return CallbackId(1); },
                                    [](CallbackId) {},
                                    recv));

            HoldsServer holder;
            holder.server_ = websrv.get();

            std::stringstream ss;
            PrintJsonSingleton::instance()->print(holder, &ss);
            Json::Value root = parse_json(ss.str());
            Json::Value const & eps = root["server"]["endpoints"];
            REQUIRE(eps.size() == 2);

            /* http /status: none */
            REQUIRE(eps[0]["receiver"].isNull());

            /* stream /fw: the receiver, by its own type */
            Json::Value const & r = eps[1]["receiver"];
            REQUIRE(r.isObject());
            REQUIRE(r["_canonical_type_"].asString() == std::string(xo::reflect::type_name<NamedReceiver>()));
            REQUIRE(r["_short_type_"].asString() == "NamedReceiver");
            REQUIRE(r["_name_"].asString() == "NamedReceiver");
            /* the endpoint's hold and this test's */
            REQUIRE(r["refcount"].asUInt() == 2);

            /* the receiver_ member refers to it */
            Json::Value const & m = eps[1]["_members_"];
            REQUIRE(m[7]["_name_"].asString() == "receiver_");
            REQUIRE(m[7]["_value_"]["_ref_"].asInt() == r["_id_"].asInt());
        }

        TEST_CASE("webserver-prints-as-json", "[websock][Webserver][json]")
        {
            HoldsServer::reflect_self();

            rp<Webserver> websrv = make_idle_server();
            websrv->register_http_endpoint(http_descr("/status"));
            websrv->register_stream_endpoint(stream_descr("/fw/${id}"));

            HoldsServer holder;
            holder.server_ = websrv.get();

            std::stringstream ss;
            PrintJsonSingleton::instance()->print(holder, &ss);

            Json::Value root = parse_json(ss.str());
            Json::Value const & srv = root["server"];

            INFO("json: " << ss.str());
            REQUIRE(srv["_name_"].asString() == "Webserver");
            /* the actual type, via self_tp() -- not the interface */
            REQUIRE(srv["_canonical_type_"].asString() == "xo::web::WebserverImpl");
            REQUIRE(srv["_short_type_"].asString() == "WebserverImpl");
            REQUIRE(srv["_id_"].isInt());
            REQUIRE(srv["refcount"].asUInt() >= 1);
            REQUIRE(srv["listen_port"].asInt() == 0);
            REQUIRE(srv["state"].asString() == "stopped");

            /* chosen C++ members (.xo-backlog/xo-websock/issues/13): each
             * with its declared type, and a value or why not
             */
            {
                Json::Value const & mem = srv["_members_"];

                std::vector<std::string> names;
                for (Json::Value const & m : mem) {
                    names.push_back(m["_name_"].asString());
                    REQUIRE(m["_canonical_type_"].isString());
                    REQUIRE(m["_short_type_"].isString());
                    REQUIRE((m.isMember("_value_") != m.isMember("_error_")));
                }

                REQUIRE(names == std::vector<std::string>{"ws_config_", "listen_port_", "state_",
                                                          "pjson_", "url_router_", "session_table_"});

                /* each declared type's xo-reflect metatype */
                std::vector<std::string> metatypes;
                for (Json::Value const & m : mem)
                    metatypes.push_back(m["_metatype_"].asString());

                REQUIRE(metatypes == std::vector<std::string>{"struct", "atomic", "atomic",
                                                              "pointer", "struct", "struct"});

                REQUIRE(mem[1]["_canonical_type_"].asString() == "std::atomic<int>");
                REQUIRE(mem[1]["_short_type_"].asString() == "atomic<int>");
                REQUIRE(mem[1]["_value_"].asInt() == 0);
                REQUIRE(mem[2]["_canonical_type_"].asString() == "xo::web::Runstate");
                REQUIRE(mem[2]["_value_"].asString() == "stopped");

                /* the config: a value with its own members, so the page can
                 * open it -- a default WebserverConfig
                 */
                {
                    Json::Value const & cfg = mem[0]["_value_"];
                    REQUIRE(cfg["_short_type_"].asString() == "WebserverConfig");
                    REQUIRE(!cfg.isMember("id"));   /* a value: printed here only */

                    std::vector<std::string> cnames;
                    for (Json::Value const & m : cfg["_members_"])
                        cnames.push_back(m["_name_"].asString());
                    REQUIRE(cnames == std::vector<std::string>{"port_", "tls_flag_", "host_check_flag_",
                                                               "use_retry_flag_", "mount_origin_"});
                    REQUIRE(cfg["_members_"][0]["_value_"].asInt() == 0);
                    REQUIRE(cfg["_members_"][1]["_value_"].asBool() == false);
                    REQUIRE(cfg["_members_"][4]["_value_"].asString() == "./mount-origin");
                }
                REQUIRE(mem[4]["_canonical_type_"].asString() == "xo::web::UrlRouter");
                REQUIRE(mem[4]["_value_"]["_canonical_type_"].asString() == "xo::web::UrlRouter");
                REQUIRE(mem[4]["_value_"]["_short_type_"].asString() == "UrlRouter");

                /* the url router: an id, and its maps -- stem -> a ref to
                 * the very endpoint printed in the server's list
                 */
                Json::Value const & ur = mem[4]["_value_"];
                REQUIRE(ur["_id_"].isInt());
                REQUIRE(ur["_members_"][0]["_name_"].asString() == "http_map_");
                REQUIRE(ur["_members_"][1]["_name_"].asString() == "stream_map_");

                std::size_t n_refs = 0;
                for (Json::Value const & ep : srv["endpoints"]) {
                    Json::Value const & map
                        = ur["_members_"][ep["kind"].asString() == "http" ? 0 : 1]["_value_"];

                    INFO("stem " << ep["stem"].asString());
                    REQUIRE(map[ep["stem"].asString()]["_ref_"].asInt() == ep["_id_"].asInt());
                    ++n_refs;
                }
                REQUIRE(n_refs == ur["_members_"][0]["_value_"].size()
                                  + ur["_members_"][1]["_value_"].size());

                /* the session table: an id; no session yet -- ids from 1 */
                Json::Value const & st = mem[5]["_value_"];
                REQUIRE(st["_id_"].isInt());
                REQUIRE(st["_members_"][0]["_name_"].asString() == "next_id_");
                REQUIRE(st["_members_"][0]["_value_"].asUInt64() == 1);
                REQUIRE(st["_members_"][1]["_name_"].asString() == "session_map_");
                REQUIRE(st["_members_"][1]["_value_"].isObject());
                REQUIRE(st["_members_"][1]["_value_"].empty());
            }

            /* every member a printer opts in to is printable: anywhere in
             * the output, no "_error_" (JsonMembers' "type not reflected")
             */
            {
                std::vector<std::string> errors;
                std::function<void (Json::Value const &)> walk
                    = [&walk, &errors](Json::Value const & x) {
                        if (x.isObject()) {
                            if (x.isMember("_error_"))
                                errors.push_back(x["_name_"].asString() + ": "
                                                 + x["_error_"].asString());
                            for (auto const & k : x.getMemberNames())
                                walk(x[k]);
                        } else if (x.isArray()) {
                            for (Json::Value const & y : x)
                                walk(y);
                        }
                    };
                walk(root);

                INFO("errors: " << errors.size() << (errors.empty() ? "" : " first: " + errors[0]));
                REQUIRE(errors.empty());
            }

            Json::Value const & eps = srv["endpoints"];
            REQUIRE(eps.size() == 2);
            REQUIRE(eps[0]["_name_"].asString() == "DynamicEndpoint");
            REQUIRE(eps[0]["_canonical_type_"].asString() == "xo::web::DynamicEndpoint");
            REQUIRE(eps[0]["kind"].asString() == "http");
            REQUIRE(eps[0]["pattern"].asString() == "/status");
            REQUIRE(eps[0]["has_receive"].asBool() == false);

            /* each endpoint's chosen C++ members (.xo-backlog/xo-websock/issues/13) */
            {
                Json::Value const & m = eps[0]["_members_"];   /* http /status */

                std::vector<std::string> names;
                for (Json::Value const & x : m)
                    names.push_back(x["_name_"].asString());

                REQUIRE(names == std::vector<std::string>{"kind_", "uri_pattern_", "uri_regex_",
                                                          "var_v_", "http_handler_", "subscribe_fn_",
                                                          "unsubscribe_fn_", "receiver_"});
                REQUIRE(m[0]["_value_"].asString() == "http");
                REQUIRE(m[1]["_value_"].asString() == "/status");
                REQUIRE(m[2]["_value_"].asString() == "0 captures");
                REQUIRE(m[3]["_value_"].isArray());
                REQUIRE(m[3]["_value_"].empty());
                REQUIRE(m[4]["_value_"].asString() == "set");
                REQUIRE(m[5]["_value_"].asString() == "empty");
                REQUIRE(m[7]["_value_"].isNull());

                Json::Value const & s = eps[1]["_members_"];    /* the stream endpoint */
                REQUIRE(s[0]["_value_"].asString() == "stream");
                REQUIRE(s[1]["_value_"].asString() == "/fw/${id}");
                REQUIRE(s[2]["_value_"].asString() == "1 captures");
                REQUIRE(s[3]["_value_"].size() == 1);
                REQUIRE(s[3]["_value_"][0].asString() == "id");
                REQUIRE(s[4]["_value_"].asString() == "empty");
                REQUIRE(s[5]["_value_"].asString() == "set");
                REQUIRE(s[6]["_value_"].asString() == "set");
            }
            REQUIRE(eps[1]["kind"].asString() == "stream");
            REQUIRE(eps[1]["stem"].asString() == "/fw/");
            REQUIRE(eps[1]["pattern"].asString() == "/fw/${id}");
            /* identity and refcount: distinct objects, each held only by the
             * router's map (no subscriptions on an idle server)
             */
            REQUIRE(eps[0]["_id_"].isInt());
            REQUIRE(eps[0]["_id_"].asInt() != eps[1]["_id_"].asInt());
            REQUIRE(eps[0]["refcount"].asUInt() == 1);
            REQUIRE(eps[1]["refcount"].asUInt() == 1);

            REQUIRE(srv["sessions"].isArray());
            REQUIRE(srv["sessions"].empty());
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end Webserver.test.cpp */
