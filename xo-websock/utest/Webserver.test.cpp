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
#include <xo/indentlog2/print/TempFlatSink.hpp>
#include <xo/ppsink/PpStyle.hpp>
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
             *  a Webserver*.  PrintJson follows the pointer and prints the
             *  Webserver from its reflection
             **/
            struct HoldsServer {
                static void reflect_self() {
                    StructReflector<HoldsServer> sr;

                    /* owning: in these tests the holder is where the
                     * server prints.  A raw pointer is borrowed by default,
                     * printing only a ref (.xo-backlog/xo-printjson/issues/08)
                     */
                    if (sr.is_incomplete())
                        REFLECT_MEMBER(sr, server).owning();
                }

                Webserver * server_ = nullptr;
            };

            /** the value of member @p name of a reflected struct's json:
             *  its "_members_" entry (.xo-backlog/xo-printjson/issues/07)
             **/
            Json::Value const & member_value(Json::Value const & obj, std::string const & name) {
                for (Json::Value const & m : obj["_members_"])
                    if (m["_name_"].asString() == name)
                        return m["_value_"];
                static Json::Value const none;
                return none;
            }

            /** the server's endpoints, as @p srv (a server's json) prints
             *  them: in full in url_router_'s maps -- http_map_, then
             *  stream_map_, each in stem order -- at first encounter.  No
             *  "endpoints" list (xo-websock#15)
             **/
            Json::Value endpoints_of(Json::Value const & srv) {
                Json::Value const & ur = member_value(srv, "url_router");
                Json::Value v(Json::arrayValue);
                for (char const * map : {"http_map", "stream_map"}) {
                    Json::Value const & m = member_value(ur, map);
                    for (std::string const & stem : m.getMemberNames())   /* jsoncpp: sorted */
                        v.append(m[stem]);
                }
                return v;
            }

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
            /* a stream receiver; SelfTagging, reflected with no members */
            class NamedReceiver : public xo::web::StreamReceiver {
            public:
                /* a StreamReceiver is SelfTagging; reflected (no members) so it
                 * prints from reflection (.xo-backlog/xo-websock/issues/16)
                 */
                static void reflect_self() { xo::reflect::StructReflector<NamedReceiver> sr; }
                xo::reflect::TaggedRcptr self_tp() override { reflect_self(); return Reflect::make_rctp(this); }
                void receive(rp<WebsocketSink> const &, Json::Value const &) override {}
            };
        }

        TEST_CASE("webserver-json-names-each-receiver", "[websock][Webserver][json]")
        {
            /* an endpoint's receiver printed in full: named by its most-
             * derived type (StreamReceiver is SelfTagging), with the id the
             * receiver member's ref uses
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

            xo::pp::TempFlatSink ss(xo::pp::PpStyle::json());
            PrintJsonSingleton::instance()->print(holder, &ss.sink());
            Json::Value root = parse_json(std::string(ss.output()));
            Json::Value const eps = endpoints_of(member_value(root, "server"));
            REQUIRE(eps.size() == 2);

            /* no inline "receiver" key: the receiver prints in full in the
             * reflected receiver member (shared), as its most-derived type
             * (xo-websock#16)
             */
            REQUIRE(!eps[1].isMember("receiver"));

            /* http /status: none */
            REQUIRE(member_value(eps[0], "receiver").isNull());

            /* stream /fw: the receiver, by its own type, placed here */
            Json::Value const & r = member_value(eps[1], "receiver");
            REQUIRE(r.isObject());
            REQUIRE(r["_id_"].isInt());
            REQUIRE(r["_canonical_type_"].asString() == std::string(xo::reflect::type_name<NamedReceiver>()));
            REQUIRE(r["_short_type_"].asString() == "NamedReceiver");
            REQUIRE(r["_name_"].asString() == "NamedReceiver");
            REQUIRE(!r.isMember("refcount"));
        }

        TEST_CASE("webserver-prints-as-json", "[websock][Webserver][json]")
        {
            HoldsServer::reflect_self();

            rp<Webserver> websrv = make_idle_server();
            websrv->register_http_endpoint(http_descr("/status"));
            websrv->register_stream_endpoint(stream_descr("/fw/${id}"));

            HoldsServer holder;
            holder.server_ = websrv.get();

            xo::pp::TempFlatSink ss(xo::pp::PpStyle::json());
            PrintJsonSingleton::instance()->print(holder, &ss.sink());

            Json::Value root = parse_json(std::string(ss.output()));
            Json::Value const & srv = member_value(root, "server");

            INFO("json: " << std::string(ss.output()));
            /* the actual type, via self_tp() -- not the interface.  Printed
             * by the generic printer, from reflection: _name_ is its short
             * name (xo-websock#16)
             */
            REQUIRE(srv["_name_"].asString() == "WebserverImpl");
            REQUIRE(srv["_canonical_type_"].asString() == "xo::web::WebserverImpl");
            REQUIRE(srv["_short_type_"].asString() == "WebserverImpl");
            REQUIRE(srv["_id_"].isInt());
            /* no view-model keys: listen_port, state are members --
             * reflected names, no trailing underscore (xo-websock#16)
             */
            REQUIRE(!srv.isMember("refcount"));
            REQUIRE(!srv.isMember("listen_port"));
            REQUIRE(!srv.isMember("state"));
            REQUIRE(member_value(srv, "listen_port").asInt() == 0);
            REQUIRE(member_value(srv, "state").asString() == "stopped");

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

                /* reflected members, unguarded first; state_ last, read
                 * under the server's mutex_ (xo-printjson#09, xo-websock#15)
                 */
                REQUIRE(names == std::vector<std::string>{"ws_config", "pjson", "url_router",
                                                          "session_table", "listen_port", "state"});

                /* an entry by name: robust to member order */
                auto entry_of = [](Json::Value const & m, std::string const & name) -> Json::Value const & {
                    for (Json::Value const & x : m)
                        if (x["_name_"].asString() == name)
                            return x;
                    static Json::Value const none;
                    return none;
                };

                /* each declared type's xo-reflect metatype */
                std::vector<std::string> metatypes;
                for (Json::Value const & m : mem)
                    metatypes.push_back(m["_metatype_"].asString());

                REQUIRE(metatypes == std::vector<std::string>{"struct", "pointer", "struct",
                                                              "struct", "atomic", "atomic"});
                /* state_: a reflected enum (xo-reflect#06) -- printed by name */

                REQUIRE(entry_of(mem, "listen_port")["_canonical_type_"].asString() == "std::atomic<int>");
                REQUIRE(entry_of(mem, "listen_port")["_short_type_"].asString() == "atomic<int>");
                REQUIRE(entry_of(mem, "listen_port")["_value_"].asInt() == 0);
                REQUIRE(entry_of(mem, "state")["_canonical_type_"].asString() == "xo::web::Runstate");
                REQUIRE(entry_of(mem, "state")["_value_"].asString() == "stopped");

                /* the config: a value with its own members, so the page can
                 * open it -- a default WebserverConfig
                 */
                {
                    Json::Value const & cfg = entry_of(mem, "ws_config")["_value_"];
                    REQUIRE(cfg["_short_type_"].asString() == "WebserverConfig");
                    REQUIRE(!cfg.isMember("id"));   /* a value: printed here only */

                    std::vector<std::string> cnames;
                    for (Json::Value const & m : cfg["_members_"])
                        cnames.push_back(m["_name_"].asString());
                    REQUIRE(cnames == std::vector<std::string>{"port", "tls_flag", "host_check_flag",
                                                               "use_retry_flag", "mount_origin"});
                    REQUIRE(cfg["_members_"][0]["_value_"].asInt() == 0);
                    REQUIRE(cfg["_members_"][1]["_value_"].asBool() == false);
                    REQUIRE(cfg["_members_"][4]["_value_"].asString() == "./mount-origin");
                }
                REQUIRE(entry_of(mem, "url_router")["_canonical_type_"].asString() == "xo::web::UrlRouter");
                REQUIRE(entry_of(mem, "url_router")["_value_"]["_canonical_type_"].asString() == "xo::web::UrlRouter");
                REQUIRE(entry_of(mem, "url_router")["_value_"]["_short_type_"].asString() == "UrlRouter");

                /* the url router: an id, and its maps -- stem -> that
                 * endpoint in full, placed here (no "endpoints" list,
                 * xo-websock#15)
                 */
                Json::Value const & ur = entry_of(mem, "url_router")["_value_"];
                REQUIRE(ur["_id_"].isInt());
                REQUIRE(ur["_members_"][0]["_name_"].asString() == "http_map");
                REQUIRE(ur["_members_"][1]["_name_"].asString() == "stream_map");

                REQUIRE(!srv.isMember("endpoints"));
                for (Json::ArrayIndex i = 0; i < 2; ++i) {
                    Json::Value const & map = ur["_members_"][i]["_value_"];

                    for (std::string const & stem : map.getMemberNames()) {
                        Json::Value const & ep = map[stem];

                        INFO("stem " << stem);
                        REQUIRE(ep["_name_"].asString() == "DynamicEndpoint");
                        REQUIRE(ep["_id_"].isInt());
                        REQUIRE(!ep.isMember("stem"));   /* the map key is the stem (xo-websock#16) */
                        REQUIRE(member_value(ep, "kind").asString() == (i == 0 ? "http" : "stream"));
                    }
                }
                REQUIRE(endpoints_of(srv).size()
                        == ur["_members_"][0]["_value_"].size() + ur["_members_"][1]["_value_"].size());

                /* the session table: an id; no session yet -- ids from 1 */
                Json::Value const & st = entry_of(mem, "session_table")["_value_"];
                REQUIRE(st["_id_"].isInt());
                REQUIRE(st["_members_"][0]["_name_"].asString() == "next_id");
                REQUIRE(st["_members_"][0]["_value_"].asUInt64() == 1);
                REQUIRE(st["_members_"][1]["_name_"].asString() == "session_map");
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

            Json::Value const eps = endpoints_of(srv);
            REQUIRE(eps.size() == 2);
            REQUIRE(eps[0]["_name_"].asString() == "DynamicEndpoint");
            REQUIRE(eps[0]["_canonical_type_"].asString() == "xo::web::DynamicEndpoint");
            /* no view-model keys (refcount, kind, stem, pattern,
             * has_receive): members (xo-websock#16)
             */
            for (char const * k : {"refcount", "kind", "stem", "pattern", "has_receive", "receiver"})
                REQUIRE(!eps[0].isMember(k));
            REQUIRE(member_value(eps[0], "kind").asString() == "http");
            REQUIRE(member_value(eps[0], "uri_pattern").asString() == "/status");
            REQUIRE(member_value(eps[0], "receiver").isNull());

            /* each endpoint's chosen C++ members (.xo-backlog/xo-websock/issues/13) */
            {
                Json::Value const & m = eps[0]["_members_"];   /* http /status */

                std::vector<std::string> names;
                for (Json::Value const & x : m)
                    names.push_back(x["_name_"].asString());

                /* every member reflected, in reflection order, named
                 * without the trailing '_'; the std::functions print
                 * "set" / "empty" (xo-reflect#09).  No uri_regex_: a
                 * compiled cache of uri_pattern_ (xo-websock#16)
                 */
                REQUIRE(names == std::vector<std::string>{"kind", "uri_pattern", "var_v",
                                                          "receiver", "http_handler",
                                                          "subscribe_fn", "unsubscribe_fn"});

                /* by name: robust to member order */
                auto value_of = [](Json::Value const & mem, std::string const & name) {
                    for (Json::Value const & x : mem)
                        if (x["_name_"].asString() == name)
                            return x["_value_"];
                    return Json::Value();
                };

                REQUIRE(value_of(m, "kind").asString() == "http");
                REQUIRE(value_of(m, "uri_pattern").asString() == "/status");
                REQUIRE(value_of(m, "var_v").isArray());
                REQUIRE(value_of(m, "var_v").empty());
                REQUIRE(value_of(m, "http_handler").asString() == "set");
                REQUIRE(value_of(m, "subscribe_fn").asString() == "empty");
                REQUIRE(value_of(m, "receiver").isNull());

                Json::Value const & s = eps[1]["_members_"];    /* the stream endpoint */
                REQUIRE(value_of(s, "kind").asString() == "stream");
                REQUIRE(value_of(s, "uri_pattern").asString() == "/fw/${id}");
                REQUIRE(value_of(s, "var_v").size() == 1);
                REQUIRE(value_of(s, "var_v")[0].asString() == "id");
                REQUIRE(value_of(s, "http_handler").asString() == "empty");
                REQUIRE(value_of(s, "subscribe_fn").asString() == "set");
                REQUIRE(value_of(s, "unsubscribe_fn").asString() == "set");
            }
            REQUIRE(member_value(eps[1], "kind").asString() == "stream");
            REQUIRE(member_value(eps[1], "uri_pattern").asString() == "/fw/${id}");
            /* identity: distinct objects (refcount no longer shown --
             * not a member)
             */
            REQUIRE(eps[0]["_id_"].isInt());
            REQUIRE(eps[0]["_id_"].asInt() != eps[1]["_id_"].asInt());

            /* no sessions: the session table's map is empty.  No "sessions"
             * list -- they print in session_table_'s session_map_
             * (xo-websock#15)
             */
            REQUIRE(!srv.isMember("sessions"));
            {
                auto member = [](Json::Value const & obj, std::string const & name) {
                    for (Json::Value const & x : obj["_members_"])
                        if (x["_name_"].asString() == name)
                            return x["_value_"];
                    return Json::Value();
                };

                Json::Value const smap = member(member(srv, "session_table"), "session_map");
                REQUIRE(smap.isObject());
                REQUIRE(smap.empty());
            }
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end Webserver.test.cpp */
