/** @file websock_json.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  json printers for xo-websock's own objects, for introspection.
 *  See .xo-backlog/xo-websock/issues/10.
 *
 *  The Webserver printer reads the server through its public API.  Every
 *  object is printed in full once, where it is owned, and elsewhere as
 *  {"ref": id} -- so neither cycles nor shared objects reach PrintJson.
 *
 *    Webserver (here)
 *     +- endpoints: DynamicEndpoint (here)
 *     +- sessions: WsSession, and its WsSessionSender (Webserver.cpp)
 *         +- subscriptions: Subscription (WsSessionRouter.cpp)
 *             +- endpoint: ref
 *             +- sink: WebsocketSink (here -> its virtual print_json)
 *                 +- sender: ref
 *
 *  Types private to one file are printed there (webserver_json.hpp).
 **/

#include "websock_json.hpp"
#include "Webserver.hpp"
#include "DynamicEndpoint.hpp"
#include "WebsocketSink.hpp"
#include "webserver_json.hpp"
#include <xo/printjson/JsonPrinter.hpp>
#include <xo/reflect/Reflect.hpp>
#include <xo/reflectutil/type_name.hpp>
#include <xo/ppsink/quoted_ostream.hpp>   /* quot(..) */
#include <cstdint>
#include <memory>
#include <sstream>

namespace xo {
    using xo::json::PrintJson;
    using xo::json::JsonPrinter;
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;
    using xo::reflect::type_name;
    using xo::pp::quot;

    namespace web {
        namespace {
            /** @brief a registered endpoint.  Printed in full where it is
             *  owned -- the Webserver's endpoint list; a subscription shows
             *  it as a ref by id
             **/
            class JsonPrinter_DynamicEndpoint : public JsonPrinter {
            public:
                JsonPrinter_DynamicEndpoint(PrintJson const * pjson) : JsonPrinter(pjson) {}

                void print_json(TaggedPtr tp, std::ostream * p_os) const override {
                    DynamicEndpoint const * ep = this->check_recover_native<DynamicEndpoint>(tp, p_os);

                    if (!ep)
                        return;

                    *p_os << "{" << quot("_name_") << ": " << quot("DynamicEndpoint")
                          << ", " << quot("_type_") << ": " << quot(type_name<DynamicEndpoint>())
                          << ", " << quot("id") << ": " << quot(json_id(ep))
                          /* held by the router's map, plus one per live
                           * subscription served (each holds it by rp<>)
                           */
                          << ", " << quot("refcount") << ": " << ep->reference_counter()
                          << ", " << quot("kind") << ": " << quot(endpoint_kind_descr(ep->kind()))
                          << ", " << quot("stem") << ": " << quot(ep->stem())
                          << ", " << quot("pattern") << ": " << quot(ep->uri_pattern())
                          << ", " << quot("has_receive") << ": " << (ep->has_receive() ? "true" : "false")
                          << "}";
                }
            }; /*JsonPrinter_DynamicEndpoint*/

            /** @brief a sink, keyed on the abstract type: delegates to its
             *  virtual print_json, so each implementation says what it holds
             **/
            class JsonPrinter_WebsocketSink : public JsonPrinter {
            public:
                JsonPrinter_WebsocketSink(PrintJson const * pjson) : JsonPrinter(pjson) {}

                void print_json(TaggedPtr tp, std::ostream * p_os) const override {
                    WebsocketSink const * sink = this->check_recover_native<WebsocketSink>(tp, p_os);

                    if (sink)
                        sink->print_json(*(this->pjson()), p_os);
                }
            };

            /** @brief Webserver, keyed on the abstract type: what a
             *  Webserver* in a reflected struct dispatches to
             **/
            class JsonPrinter_Webserver : public JsonPrinter {
            public:
                JsonPrinter_Webserver(PrintJson const * pjson) : JsonPrinter(pjson) {}

                void print_json(TaggedPtr tp, std::ostream * p_os) const override {
                    Webserver const * websrv = this->check_recover_native<Webserver>(tp, p_os);

                    if (!websrv)
                        return;

                    std::string const type
                        = const_cast<Webserver *>(websrv)->self_tp().td()->canonical_name();

                    *p_os << "{" << quot("_name_") << ": " << quot("Webserver")
                          << ", " << quot("_type_") << ": " << quot(type)
                          << ", " << quot("id") << ": " << quot(json_id(websrv))
                          << ", " << quot("refcount") << ": " << websrv->reference_counter()
                          << ", " << quot("listen_port") << ": " << websrv->listen_port()
                          << ", " << quot("state") << ": "
                          << quot(RunstateUtil::runstate_descr(websrv->state()));

                    *p_os << ", " << quot("endpoints") << ": [";
                    {
                        bool first = true;
                        PrintJson const * pjson = this->pjson();

                        /* under the router's lock; printing never calls back
                         * into the router
                         */
                        websrv->visit_endpoints([pjson, p_os, &first](DynamicEndpoint const & ep) {
                                if (!first)
                                    *p_os << ", ";
                                first = false;

                                pjson->print_aux(TaggedPtr(Reflect::require<DynamicEndpoint>(),
                                                           const_cast<DynamicEndpoint *>(&ep)),
                                                 p_os);
                            });
                    }
                    *p_os << "]";

                    *p_os << ", " << quot("sessions") << ": [";
                    {
                        bool first = true;
                        PrintJson const * pjson = this->pjson();

                        /* each session via its own printer (installed by
                         * provide_webserver_json_printers), in id order
                         */
                        websrv->visit_sessions([pjson, p_os, &first](TaggedPtr session) {
                                if (!first)
                                    *p_os << ", ";
                                first = false;

                                pjson->print_aux(session, p_os);
                            });
                    }
                    *p_os << "]";

                    *p_os << "}";
                }
            }; /*JsonPrinter_Webserver*/
        } /*namespace*/

        void
        provide_websock_json_printers(PrintJson * pjson)
        {
            pjson->provide_printer(Reflect::require<Webserver>(),
                                   std::make_unique<JsonPrinter_Webserver>(pjson));
            pjson->provide_printer(Reflect::require<DynamicEndpoint>(),
                                   std::make_unique<JsonPrinter_DynamicEndpoint>(pjson));
            pjson->provide_printer(Reflect::require<WebsocketSink>(),
                                   std::make_unique<JsonPrinter_WebsocketSink>(pjson));
            /* types private to one file, printed there */
            provide_webserver_json_printers(pjson);
            provide_router_json_printers(pjson);
        }
    } /*namespace web*/
} /*namespace xo*/

/* end websock_json.cpp */
