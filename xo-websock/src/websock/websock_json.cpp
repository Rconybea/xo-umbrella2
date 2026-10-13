/** @file websock_json.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  json printers for xo-websock's own objects, for introspection.
 *  See .xo-backlog/xo-websock/issues/10.
 *
 *  One bespoke printer is left: DynamicEndpoint's.  Every other websock
 *  type prints from reflection, with the generic printer
 *  (.xo-backlog/xo-websock/issues/16).  Each object prints in full once --
 *  where an owning or shared edge first reaches it -- and elsewhere as
 *  {"_ref_": id} (.xo-backlog/xo-printjson/issues/08):
 *
 *    WebserverImpl (a Webserver* is reflected as its actual type)
 *     +- url_router: UrlRouter
 *     |   +- http_map, stream_map: DynamicEndpoint (here)
 *     +- session_table: WsSessionTable
 *         +- session_map: WebsocketSessionRecd
 *             +- router: WsSessionRouter
 *             |   +- subscription_v: Subscription
 *             |       +- endpoint: ref
 *             |       +- sink: WebsocketSinkImpl
 *             |           +- sender: ref
 *             +- sender: WsSessionSender
 **/

#include "websock_json.hpp"
#include "Webserver.hpp"
#include "DynamicEndpoint.hpp"
#include "WebsocketSink.hpp"
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
        /** @brief a registered endpoint.  Printed in full where first
         *  reached -- its url router's map (a shared edge); a subscription
         *  shows it as a ref by id.  Not in the anonymous namespace:
         *  DynamicEndpoint's header befriends it by name, for "_members_"
         **/
        class JsonPrinter_DynamicEndpoint : public JsonPrinter {
        public:
            void print_json(TaggedPtr tp, json::JsonPrintState & state) const override {
                DynamicEndpoint const * ep = this->check_recover_native<DynamicEndpoint>(tp, state);

                if (!ep)
                    return;

                /* the receiver: written inline below, at its most-derived
                 * address -- the address the reflected receiver_ member
                 * reaches through self_tp(), so it prints as a ref to this
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
                 * the reflected ones (kind_, uri_pattern_, var_v_,
                 * receiver_ -- a ref: the receiver printed in full above),
                 * then the rest.  Not printable as themselves -- the
                 * compiled regex, the std::functions -- so their capture
                 * count, presence, under their declared types
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
                    .end();

                obj.close();
            }
        }; /*JsonPrinter_DynamicEndpoint*/

        void
        provide_websock_json_printers(PrintJson * pjson)
        {
            /* every other websock type has no printer -- the generic one,
             * from reflection (.xo-backlog/xo-websock/issues/16)
             */
            pjson->provide_printer(Reflect::require<DynamicEndpoint>(),
                                   std::make_unique<JsonPrinter_DynamicEndpoint>());
        }
    } /*namespace web*/
} /*namespace xo*/

/* end websock_json.cpp */
