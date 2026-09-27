/** @file websock_json.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  json printers for xo-websock's own objects, for introspection.
 *  See .xo-backlog/xo-websock/issues/10.
 *
 *  Increment 5a: the Webserver printer reads the server through its public
 *  listings -- endpoints(), sessions() -- and prints those Info values
 *  inline.  Later increments replace each Info fragment with a printer for
 *  the native object (DynamicEndpoint, the session, WsSessionSender, the
 *  subscription and its sink), after which the Info types go.
 **/

#include "websock_json.hpp"
#include "Webserver.hpp"
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

            void print_endpoint_info(EndpointInfo const & x, std::ostream * p_os) {
                *p_os << "{" << quot("kind") << ": " << quot(endpoint_kind_descr(x.kind_))
                      << ", " << quot("stem") << ": " << quot(x.stem_)
                      << ", " << quot("pattern") << ": " << quot(x.uri_pattern_)
                      << "}";
            }

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
                        for (EndpointInfo const & ep : websrv->endpoints()) {
                            if (!first)
                                *p_os << ", ";
                            first = false;

                            print_endpoint_info(ep, p_os);
                        }
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
        }
    } /*namespace web*/
} /*namespace xo*/

/* end websock_json.cpp */
