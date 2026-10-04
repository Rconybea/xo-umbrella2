/** @file websock_json.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  json printers for xo-websock's own objects, for introspection.
 *  See .xo-backlog/xo-websock/issues/10.
 *
 *  Every object is printed in full once, where it is owned, and elsewhere
 *  as {"ref": id} -- so neither cycles nor shared objects reach PrintJson.
 *
 *    WebserverImpl (Webserver.cpp: a Webserver* is reflected as its actual
 *                   type)
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
#include <xo/printjson/JsonMembers.hpp>
#include <xo/printjson/type_keys.hpp>
#include <xo/reflect/Reflect.hpp>
#include <xo/ppsink/quoted_ostream.hpp>   /* quot(..) */
#include <cstdint>
#include <memory>
#include <sstream>

namespace xo {
    using xo::json::PrintJson;
    using xo::json::JsonPrinter;
    using xo::json::JsonMembers;
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TaggedRcptr;
    using xo::pp::quot;

    namespace web {
        /** @brief a registered endpoint.  Printed in full where it is
         *  owned -- the Webserver's endpoint list; a subscription shows
         *  it as a ref by id.  Not in the anonymous namespace:
         *  DynamicEndpoint's header befriends it by name, for "_members_"
         **/
        class JsonPrinter_DynamicEndpoint : public JsonPrinter {
        public:
            JsonPrinter_DynamicEndpoint(PrintJson const * pjson) : JsonPrinter(pjson) {}

            void print_json(TaggedPtr tp, std::ostream * p_os) const override {
                DynamicEndpoint const * ep = this->check_recover_native<DynamicEndpoint>(tp, p_os);

                if (!ep)
                    return;

                *p_os << "{" << quot("_name_") << ": " << quot("DynamicEndpoint")
                      << ", " << json::type_keys(tp.td())
                      << ", " << quot("id") << ": " << quot(json_id(ep))
                      /* held by the router's map, plus one per live
                       * subscription served (each holds it by rp<>)
                       */
                      << ", " << quot("refcount") << ": " << ep->reference_counter()
                      << ", " << quot("kind") << ": " << quot(endpoint_kind_descr(ep->kind()))
                      << ", " << quot("stem") << ": " << quot(ep->stem())
                      << ", " << quot("pattern") << ": " << quot(ep->uri_pattern())
                      << ", " << quot("has_receive") << ": " << (ep->has_receive() ? "true" : "false");

                /* the receiver, printed here in full -- its identity, named
                 * by its most-derived type (SelfTagging); members later.
                 * Its "id" is what the receiver_ member's ref writes below
                 */
                *p_os << ", " << quot("receiver") << ": ";
                if (StreamReceiver * r = ep->receiver_.get()) {
                    /* before self_tp(): the TaggedRcptr it returns holds one more */
                    auto refcount = r->reference_counter();
                    TaggedRcptr self = r->self_tp();

                    *p_os << "{" << quot("_name_") << ": " << quot(self.td()->short_name())
                          << ", " << json::type_keys(self.td())
                          << ", " << quot("id") << ": " << quot(json_id(dynamic_cast<void const *>(r)))
                          /* the endpoint's hold, plus whatever the application keeps */
                          << ", " << quot("refcount") << ": " << refcount
                          << "}";
                } else {
                    *p_os << "null";
                }

                /* chosen C++ members (.xo-backlog/xo-websock/issues/13).
                 * Not printable as themselves -- the enum, the compiled
                 * regex, the std::functions -- so their names, capture
                 * count, presence, under their declared types.  The
                 * receiver is printed in full above: here a ref
                 */
                JsonMembers mem(this->pjson(), p_os);
                mem.member_as<EndpointKind>("kind_", std::string(endpoint_kind_descr(ep->kind_)))
                    .member("uri_pattern_", ep->uri_pattern_)
                    .member_as<std::regex>("uri_regex_",
                                           std::to_string(ep->uri_regex_.mark_count()) + " captures")
                    .member("var_v_", ep->var_v_)
                    .member_as<HttpHandler>("http_handler_",
                                            std::string(ep->http_handler_ ? "set" : "empty"))
                    .member_as<StreamSubscribeFn>("subscribe_fn_",
                                                  std::string(ep->subscribe_fn_ ? "set" : "empty"))
                    .member_as<StreamUnsubscribeFn>("unsubscribe_fn_",
                                                    std::string(ep->unsubscribe_fn_ ? "set" : "empty"))
                    .member_ref<rp<StreamReceiver>>("receiver_",
                                                    dynamic_cast<void const *>(ep->receiver_.get()));
                mem.end();

                *p_os << "}";
            }
        }; /*JsonPrinter_DynamicEndpoint*/

        namespace {
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

        } /*namespace*/

        void
        provide_websock_json_printers(PrintJson * pjson)
        {
            pjson->provide_printer(Reflect::require<DynamicEndpoint>(),
                                   std::make_unique<JsonPrinter_DynamicEndpoint>(pjson));
            pjson->provide_printer(Reflect::require<WebsocketSink>(),
                                   std::make_unique<JsonPrinter_WebsocketSink>(pjson));
            /* types private to one file, printed there -- including the
             * server: a Webserver* is reflected as its actual type,
             * WebserverImpl (SelfTaggingDisplayable)
             */
            provide_webserver_json_printers(pjson);
            provide_router_json_printers(pjson);
            provide_url_router_json_printers(pjson);
        }
    } /*namespace web*/
} /*namespace xo*/

/* end websock_json.cpp */
