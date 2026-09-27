/** @file websock_json.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  json printers for xo-websock's own objects, for introspection.
 *  See .xo-backlog/xo-websock/issues/10.
 *
 *  The Webserver printer reads the server through its public API.  Native
 *  printers so far: Webserver, DynamicEndpoint (5b).  Sessions are still
 *  printed from the SessionInfo listing; later increments replace that with
 *  printers for the native objects (the session, WsSessionSender, the
 *  subscription and its sink), after which the Info types go.
 **/

#include "websock_json.hpp"
#include "Webserver.hpp"
#include "DynamicEndpoint.hpp"
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

            void print_subscription_info(SubscriptionInfo const & x, std::ostream * p_os) {
                *p_os << "{" << quot("sub_id") << ": " << x.sub_id_
                      << ", " << quot("stream") << ": " << quot(x.stream_name_)
                      << ", " << quot("endpoint") << ": " << quot(x.endpoint_pattern_)
                      << "}";
            }

            void print_session_info(SessionInfo const & x, std::ostream * p_os) {
                *p_os << "{" << quot("id") << ": " << x.session_id_
                      << ", " << quot("sender_open") << ": " << (x.sender_open_ ? "true" : "false")
                      << ", " << quot("subscriptions") << ": [";

                bool first = true;
                for (SubscriptionInfo const & sub : x.subscriptions_) {
                    if (!first)
                        *p_os << ", ";
                    first = false;

                    print_subscription_info(sub, p_os);
                }

                *p_os << "]}";
            }

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
                        for (SessionInfo const & s : websrv->sessions()) {
                            if (!first)
                                *p_os << ", ";
                            first = false;

                            print_session_info(s, p_os);
                        }
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
        }
    } /*namespace web*/
} /*namespace xo*/

/* end websock_json.cpp */
