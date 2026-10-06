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
#include <xo/printjson/JsonObject.hpp>
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
        /** a WebserverConfig: a value, printed inside its Webserver --
         *  no box of its own; its members, so the page can open it
         **/
        class JsonPrinter_WebserverConfig : public JsonPrinter {
        public:
            void print_json(TaggedPtr tp, json::JsonPrintState & state) const override {
                WebserverConfig const * cfg = this->check_recover_native<WebserverConfig>(tp, state);

                if (!cfg)
                    return;

                json::JsonObject obj = state.open_object("WebserverConfig", tp.td());

                /* reflected as port, ..: "_members_" names C++ members, port_ */
                obj.members()
                    .reflected_members(tp, "_")
                    .end();

                obj.close();
            }
        }; /*JsonPrinter_WebserverConfig*/

        class JsonPrinter_DynamicEndpoint : public JsonPrinter {
        public:
            void print_json(TaggedPtr tp, json::JsonPrintState & state) const override {
                DynamicEndpoint const * ep = this->check_recover_native<DynamicEndpoint>(tp, state);

                if (!ep)
                    return;

                /* the receiver: written inline below, at its most-derived
                 * address; the receiver_ member's ref names the same one
                 */
                StreamReceiver * r = ep->receiver_.get();
                void const * r_addr = dynamic_cast<void const *>(r);

                json::JsonObject obj = state.open_object("DynamicEndpoint", tp.td());

                /* refcount: held by the router's map, plus one per live
                 * subscription served (each holds it by rp<>)
                 */
                obj.key("refcount", ep->reference_counter())
                    .key("kind", std::string(endpoint_kind_descr(ep->kind())))
                    .key("stem", ep->stem())
                    .key("pattern", ep->uri_pattern())
                    .key("has_receive", ep->has_receive());

                /* the receiver, printed here in full -- its identity, named
                 * by its most-derived type (SelfTagging), and its reflected
                 * members
                 */
                std::ostream & os = obj.key_open("receiver");
                if (!r) {
                    os << "null";
                } else if (state.is_printed(r_addr)) {
                    state.print_ref(r_addr);
                } else {
                    /* before self_tp(): the TaggedRcptr it returns holds one more */
                    auto refcount = r->reference_counter();
                    TaggedRcptr self = r->self_tp();

                    json::JsonObject robj = state.open_object_at(r_addr, self.td()->short_name(),
                                                                 self.td());
                    /* the endpoint's hold, plus whatever the application keeps */
                    robj.key("refcount", refcount);
                    /* whatever its actual type reflects -- through self_tp(),
                     * so an application's receiver shows its own members
                     */
                    robj.members()
                        .reflected_members(self, "_")
                        .end();
                    robj.close();
                }

                /* chosen C++ members (.xo-backlog/xo-websock/issues/13):
                 * the reflected ones (kind_, uri_pattern_, var_v_), then the rest.
                 * Not printable as themselves -- the enum, the compiled
                 * regex, the std::functions -- so their names, capture
                 * count, presence, under their declared types.  The
                 * receiver is printed in full above: here a ref
                 */
                obj.members()
                    .reflected_members(tp, "_")
                    .member_as<std::regex>("uri_regex_",
                                           std::to_string(ep->uri_regex_.mark_count()) + " captures")
                    .member_as<HttpHandler>("http_handler_",
                                            std::string(ep->http_handler_ ? "set" : "empty"))
                    .member_as<StreamSubscribeFn>("subscribe_fn_",
                                                  std::string(ep->subscribe_fn_ ? "set" : "empty"))
                    .member_as<StreamUnsubscribeFn>("unsubscribe_fn_",
                                                    std::string(ep->unsubscribe_fn_ ? "set" : "empty"))
                    .member_ref<rp<StreamReceiver>>("receiver_", r_addr)
                    .end();

                obj.close();
            }
        }; /*JsonPrinter_DynamicEndpoint*/

        namespace {
            /** @brief a sink, keyed on the abstract type: delegates to its
             *  virtual print_json, so each implementation says what it holds
             **/
            class JsonPrinter_WebsocketSink : public JsonPrinter {
            public:
                void print_json(TaggedPtr tp, json::JsonPrintState & state) const override {
                    WebsocketSink const * sink = this->check_recover_native<WebsocketSink>(tp, state);

                    if (sink)
                        sink->print_json(state);
                }
            };

        } /*namespace*/

        void
        provide_websock_json_printers(PrintJson * pjson)
        {
            pjson->provide_printer(Reflect::require<WebserverConfig>(),
                                   std::make_unique<JsonPrinter_WebserverConfig>());
            pjson->provide_printer(Reflect::require<DynamicEndpoint>(),
                                   std::make_unique<JsonPrinter_DynamicEndpoint>());
            pjson->provide_printer(Reflect::require<WebsocketSink>(),
                                   std::make_unique<JsonPrinter_WebsocketSink>());
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
