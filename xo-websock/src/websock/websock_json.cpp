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

                json::JsonObject obj = state.open_object("DynamicEndpoint", tp.td());

                /* the reflected members (kind_, uri_pattern_, var_v_,
                 * receiver_ -- shared: the receiver prints in full there, as
                 * its most-derived type), then the rest.  No view-model keys
                 * (.xo-backlog/xo-websock/issues/16).  This printer stays only
                 * for the three std::functions, not printable as themselves:
                 * their presence -- until .xo-backlog/xo-reflect/issues/09.
                 * Not uri_regex_: a compiled cache of uri_pattern_, printed
                 * already (RC, 2026-10-12)
                 */
                obj.members()
                    .reflected_members(tp, "_")
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
