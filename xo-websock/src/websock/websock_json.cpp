/** @file websock_json.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  json printers for xo-websock's own objects, for introspection.
 *  See .xo-backlog/xo-websock/issues/10.
 *
 *  The Webserver printer reads the server through its public API.  Native
 *  printers so far: Webserver, DynamicEndpoint (5b); the session and its
 *  WsSessionSender (5c) live in Webserver.cpp, which alone sees those types
 *  (webserver_json.hpp).  A session's subscriptions are still printed from
 *  SubscriptionInfo; 5d replaces that with the subscription and its sink.
 **/

#include "websock_json.hpp"
#include "Webserver.hpp"
#include "DynamicEndpoint.hpp"
#include "webserver_json.hpp"
#include <xo/printjson/JsonPrinter.hpp>
#include <xo/reflect/Reflect.hpp>
#include <xo/ppsink/quoted_ostream.hpp>   /* quot(..) */
#include <cstdint>
#include <memory>
#include <sstream>

namespace xo {
    using xo::json::PrintJson;
    using xo::json::JsonPrinter;
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;
    using xo::pp::quot;

    namespace web {
        namespace {
            /** an object's identity on the page: its address, as a json
             *  string.  Unique within one snapshot; an address may be reused
             *  once its object is freed, so not across snapshots
             **/
            std::string json_id(void const * p) {
                std::ostringstream ss;
                ss << p;
                return ss.str();
            }

            /* ----- temporary: Info fragments, retired as native printers
             *       arrive (increments 5b..5d)
             */

            /** @brief a registered endpoint.  Printed in full where it is
             *  owned -- the Webserver's endpoint list; elsewhere (a
             *  subscription, 5d) it will appear as a ref by id
             **/
            class JsonPrinter_DynamicEndpoint : public JsonPrinter {
            public:
                JsonPrinter_DynamicEndpoint(PrintJson const * pjson) : JsonPrinter(pjson) {}

                void print_json(TaggedPtr tp, std::ostream * p_os) const override {
                    DynamicEndpoint const * ep = this->check_recover_native<DynamicEndpoint>(tp, p_os);

                    if (!ep)
                        return;

                    *p_os << "{" << quot("_name_") << ": " << quot("DynamicEndpoint")
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

                    *p_os << "{" << quot("_name_") << ": " << quot("Webserver")
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
            /* session and sender: private to Webserver.cpp, printed there */
            provide_webserver_json_printers(pjson);
        }
    } /*namespace web*/
} /*namespace xo*/

/* end websock_json.cpp */
