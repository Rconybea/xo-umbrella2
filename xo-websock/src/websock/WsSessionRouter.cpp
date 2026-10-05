/** @file WsSessionRouter.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "WsSessionRouter.hpp"
#include "UrlRouter.hpp"
#include "DynamicEndpoint.hpp"
#include "webserver_json.hpp"
#include <xo/printjson/JsonPrinter.hpp>
#include <xo/printjson/JsonMembers.hpp>
#include <xo/printjson/JsonObject.hpp>
#include <xo/printjson/type_keys.hpp>
#include <xo/reflect/Reflect.hpp>
#include <xo/ppsink/quoted_ostream.hpp>   /* quot(..) */
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag_ostream.hpp>      /* xtag(..) */
#include <xo/reflect/StructReflector.hpp>
#include <json/json.h>
#include <exception>

namespace xo {
    using xo::fn::CallbackId;
    using xo::json::PrintJson;
    using xo::json::JsonPrinter;
    using xo::json::JsonMembers;
    using xo::reflect::Reflect;
    using xo::reflect::StructReflector;
    using xo::reflect::TaggedPtr;
    using xo::pp::quot;

    namespace web {
        using xo::pp::scope;
        using xo::pp::xtag;

        struct WsSessionRouter::Subscription {
            /** describe Subscription to xo-reflect; see WsSessionRouter::reflect_self **/
            static void reflect_self(reflect::TypeDescrTable * table);

            /* index in .subscription_v; what the client addresses */
            std::uint32_t sub_id_ = 0;
            /* stream name from the subscribe command */
            std::string stream_name_;
            /* endpoint serving the stream.  Held by rp, not borrowed: if the
             * stream is re-registered while this subscription lives, the map
             * lets go of this endpoint but we must not -- unsubscribe has to
             * run on the endpoint that subscribed us
             */
            rp<DynamicEndpoint> endpoint_;
            /* id from the endpoint's subscribe function, for unsubscribe */
            CallbackId callback_id_;
            /* sink delivering to this session for this stream */
            rp<WebsocketSink> sink_;
        };

        WsSessionRouter::WsSessionRouter(UrlRouter const & url_router,
                                         rp<WsSender> sender,
                                         rp<PrintJson> pjson)
            : url_router_{url_router},
              sender_{std::move(sender)},
              pjson_{std::move(pjson)},
              readjson_{Json::CharReaderBuilder().newCharReader()}
        {}

        WsSessionRouter::~WsSessionRouter() = default;

        void
        WsSessionRouter::perform_cmd(std::string_view incoming_cmd)
        {
            scope log(XO_ENTER0_(info), xtag("incoming_cmd", incoming_cmd));

            Json::Value root;
            JSONCPP_STRING err;

            if (!readjson_->parse(incoming_cmd.data(),
                                  incoming_cmd.data() + incoming_cmd.size(),
                                  &root,
                                  &err))
            {
                this->reply_error("malformed json: " + err);
                return;
            }

            /* Type checks BEFORE any field access.  jsoncpp throws
             * Json::LogicError from operator[] on a non-object and from
             * asString() on a non-string, and an exception escaping into
             * libwebsockets' C callback terminates the server.
             */
            if (!root.isObject()) {
                this->reply_error("message is not a json object");
                return;
            }

            Json::Value const & cmd_v = root["cmd"];
            std::string cmd = (cmd_v.isString() ? cmd_v.asString() : std::string());

            log && log(xtag("cmd", cmd));

            if (cmd == "subscribe") {
                Json::Value const & stream_v = root["stream"];

                if (!stream_v.isString() || stream_v.asString().empty()) {
                    this->reply_error("subscribe requires a \"stream\"");
                    return;
                }

                this->subscribe(stream_v.asString());
            } else if (cmd == "send") {
                std::uint32_t sub_id = 0;

                if (this->require_sub_id(root, "send", &sub_id)) {
                    /* msg is optional; absent reads as json null */
                    this->send(sub_id, root["msg"]);
                }
            } else if (cmd == "unsubscribe") {
                std::uint32_t sub_id = 0;

                if (this->require_sub_id(root, "unsubscribe", &sub_id))
                    this->unsubscribe(sub_id);
            } else {
                /* unknown command: logged and ignored, as before.  A deliberate
                 * scope limit -- see .xo-backlog/xo-websock/issues/04
                 */
                log && log("unknown command, ignored");
            }
        } /*perform_cmd*/

        bool
        WsSessionRouter::require_sub_id(Json::Value const & root,
                                        char const * cmd,
                                        std::uint32_t * p_sub_id)
        {
            Json::Value const & v = root["sub_id"];

            /* isUInt(): a non-negative integer that fits uint32 -- rejects
             * strings, fractions and negatives before asUInt() could throw
             */
            if (!v.isUInt()) {
                this->reply_error(std::string(cmd) + " requires a \"sub_id\"");
                return false;
            }

            *p_sub_id = v.asUInt();
            return true;
        }

        bool
        WsSessionRouter::lookup_active(std::uint32_t sub_id, Subscription * p_copy)
        {
            bool known = false;
            bool active = false;

            {
                std::lock_guard<std::mutex> lock(this->mutex_);

                if (sub_id < this->subscription_v_.size()) {
                    known = true;

                    Subscription * sub = this->subscription_v_[sub_id].get();

                    if (sub) {
                        active = true;
                        *p_copy = *sub;
                    }
                }
            }

            if (!known) {
                this->reply_error("unknown sub_id", nullptr, &sub_id);
                return false;
            }

            if (!active) {
                /* retired slot: the id existed once and is never reused, so
                 * this cannot be mistaken for a different subscription
                 */
                this->reply_error("already unsubscribed", nullptr, &sub_id);
                return false;
            }

            return true;
        }

        void
        WsSessionRouter::subscribe(std::string const & stream_name)
        {
            scope log(XO_ENTER0_(info), xtag("stream", stream_name));

            rp<DynamicEndpoint> endpoint = url_router_.find_stream(stream_name);

            if (!endpoint) {
                /* was silent until issue 06; a page that subscribes to a
                 * stream that does not exist now hears about it
                 */
                this->reply_error("unknown stream", &stream_name);
                return;
            }

            std::uint32_t sub_id = 0;
            Subscription * sub_addr = nullptr;

            {
                std::lock_guard<std::mutex> lock(this->mutex_);

                /* the index IS the id; slots are never removed, so this never
                 * repeats within a session
                 */
                sub_id = static_cast<std::uint32_t>(this->subscription_v_.size());

                std::unique_ptr<Subscription> sub(new Subscription());
                sub->sub_id_ = sub_id;
                sub->stream_name_ = stream_name;
                sub->endpoint_ = endpoint;

                sub_addr = sub.get();
                this->subscription_v_.push_back(std::move(sub));
            }

            /* lock dropped from here: making the sink, replying and
             * subscribing may all re-enter the server
             */
            rp<WebsocketSink> sink
                = WebsocketSink::make(this->sender_, this->pjson_, stream_name, sub_id);

            {
                std::lock_guard<std::mutex> lock(this->mutex_);
                sub_addr->sink_ = sink;
            }

            /* ORDER MATTERS: the client must learn its sub_id before any frame
             * carrying it arrives, and the endpoint's subscribe function may
             * send an initial frame immediately -- the flywheel demo's does.
             */
            {
                Json::Value msg(Json::objectValue);
                msg["cmd"] = "subscribed";
                msg["stream"] = stream_name;
                msg["sub_id"] = sub_id;

                this->reply(msg);
            }

            CallbackId id = endpoint->subscribe(stream_name, sink);

            {
                std::lock_guard<std::mutex> lock(this->mutex_);
                sub_addr->callback_id_ = id;
            }
        } /*subscribe*/

        void
        WsSessionRouter::send(std::uint32_t sub_id, Json::Value const & msg)
        {
            Subscription sub;

            if (!this->lookup_active(sub_id, &sub))
                return;

            if (!sub.endpoint_->has_receive()) {
                this->reply_error("stream does not accept messages",
                                  &sub.stream_name_, &sub_id);
                return;
            }

            /* lock not held: the handler is expected to send */
            try {
                sub.endpoint_->receive(sub.sink_, msg);
            } catch (std::exception & ex) {
                /* never let a handler's exception reach libwebsockets */
                this->reply_error(std::string("stream handler failed: ") + ex.what(),
                                  &sub.stream_name_, &sub_id);
            }
        } /*send*/

        void
        WsSessionRouter::unsubscribe(std::uint32_t sub_id)
        {
            Subscription sub;

            if (!this->lookup_active(sub_id, &sub))
                return;

            {
                std::lock_guard<std::mutex> lock(this->mutex_);

                /* retire the slot: null, never erased (later ids would shift)
                 * and never reused (a stale id would reach someone else)
                 */
                this->subscription_v_[sub_id].reset();
            }

            /* lock dropped, as for subscribe */
            sub.endpoint_->unsubscribe(sub.callback_id_);

            /* frames already queued may still arrive; this marks the end */
            Json::Value msg(Json::objectValue);
            msg["cmd"] = "unsubscribed";
            msg["sub_id"] = sub_id;

            this->reply(msg);
        } /*unsubscribe*/

        void
        WsSessionRouter::reply(Json::Value const & msg)
        {
            /* jsoncpp for writing too: stream names and error reasons may carry
             * arbitrary text that must be escaped
             */
            Json::StreamWriterBuilder wb;
            wb["indentation"] = "";

            this->sender_->send_text(Json::writeString(wb, msg));
        }

        void
        WsSessionRouter::reply_error(std::string const & reason,
                                     std::string const * stream_name,
                                     std::uint32_t const * sub_id)
        {
            Json::Value msg(Json::objectValue);

            msg["error"] = reason;
            if (stream_name)
                msg["stream"] = *stream_name;
            if (sub_id)
                msg["sub_id"] = *sub_id;

            this->reply(msg);
        } /*reply_error*/

        void
        WsSessionRouter::unsubscribe_all()
        {
            std::vector<std::unique_ptr<Subscription>> subs;

            {
                std::lock_guard<std::mutex> lock(this->mutex_);
                subs.swap(this->subscription_v_);
            }

            /* lock dropped, as for subscribe.  No replies: the session is
             * closing
             */
            for (auto const & sub : subs) {
                if (sub)
                    sub->endpoint_->unsubscribe(sub->callback_id_);
            }
        } /*unsubscribe_all*/

        std::size_t
        WsSessionRouter::end_subscriptions_on(rp<DynamicEndpoint> const & endpoint)
        {
            scope log(XO_ENTER0_(info));

            std::vector<Subscription> ended_v;

            {
                std::lock_guard<std::mutex> lock(this->mutex_);

                for (auto & sub : this->subscription_v_) {
                    if (sub && (sub->endpoint_.get() == endpoint.get())) {
                        ended_v.push_back(*sub);
                        /* retire, as for unsubscribe: never erased or reused */
                        sub.reset();
                    }
                }
            }

            /* lock dropped, as for unsubscribe */
            for (Subscription const & sub : ended_v) {
                sub.endpoint_->unsubscribe(sub.callback_id_);

                Json::Value msg(Json::objectValue);
                msg["cmd"] = "unsubscribed";
                msg["sub_id"] = sub.sub_id_;
                msg["reason"] = "endpoint removed";

                this->reply(msg);
            }

            log && log(xtag("n_ended", ended_v.size()));

            return ended_v.size();
        } /*end_subscriptions_on*/

        void
        WsSessionRouter::visit_subscriptions(SubscriptionVisitor const & fn) const
        {
            std::lock_guard<std::mutex> lock(this->mutex_);

            /* index order IS sub_id order */
            for (auto const & sub : this->subscription_v_) {
                if (sub)
                    fn(TaggedPtr(Reflect::require<Subscription>(), sub.get()));
            }
        } /*visit_subscriptions*/

        namespace {
            /** @brief one subscription, printed in full by its router: its
             *  sink in full (the router's slot is one of the sink's holders);
             *  the endpoint as a ref -- it is printed in full in the
             *  webserver's endpoint list
             **/
            class JsonPrinter_Subscription : public JsonPrinter {
            public:
                using Subscription = WsSessionRouter::Subscription;

                void print_json(TaggedPtr tp, json::JsonPrintState & state) const override {
                    Subscription const * sub = this->check_recover_native<Subscription>(tp, state);

                    if (!sub)
                        return;

                    /* the sink prints through its WebsocketSink view (below):
                     * a ref to it names that same address
                     */
                    WebsocketSink const * sink = sub->sink_.get();

                    json::JsonObject obj = state.open_object("Subscription", tp.td());

                    obj.key("sub_id", sub->sub_id_)
                        .key("stream", sub->stream_name_)
                        .key_ref("endpoint", sub->endpoint_.get());

                    if (sink) {
                        obj.child("sink", TaggedPtr(Reflect::require<WebsocketSink>(),
                                                    const_cast<WebsocketSink *>(sink)));
                    } else {
                        /* in the moment between slot and sink (subscribe) */
                        obj.key_open("sink") << "null";
                    }

                    /* chosen C++ members (.xo-backlog/xo-websock/issues/13):
                     * the reflected ones (sub_id_, stream_name_), then the rest.
                     * endpoint_ and sink_ are printed in full elsewhere (the
                     * server's endpoints; "sink" above): refs, each by the
                     * address it prints at.
                     * callback_id_: CallbackId is not reflected -- its number,
                     * under its declared type
                     */
                    obj.members()
                        .reflected_members(tp, "_")
                        .member_ref<rp<DynamicEndpoint>>("endpoint_", sub->endpoint_.get())
                        .member_as<CallbackId>("callback_id_", sub->callback_id_.id())
                        .member_ref<rp<WebsocketSink>>("sink_", sink)
                        .end();

                    obj.close();
                }
            };
        } /*namespace*/

        /** @brief a session's router: its chosen C++ members
         *  (.xo-backlog/xo-websock/issues/13).  Not in the anonymous
         *  namespace: WsSessionRouter's header befriends it by name.
         *
         *  Its subscriptions are printed in full by the session
         *  ("subscriptions"), and its sender too ("sender") -- refs here.
         *  url_router_ is the server's, printed inside the server.
         **/
        class JsonPrinter_WsSessionRouter : public JsonPrinter {
        public:
            void print_json(TaggedPtr tp, json::JsonPrintState & state) const override {
                WsSessionRouter const * r = this->check_recover_native<WsSessionRouter>(tp, state);

                if (!r)
                    return;

                /* slot order, released slots null: index is sub_id */
                std::vector<void const *> sub_v;
                {
                    std::lock_guard<std::mutex> lock(r->mutex_);

                    for (auto const & sub : r->subscription_v_)
                        sub_v.push_back(sub.get());
                }

                json::JsonObject obj = state.open_object("WsSessionRouter", tp.td());

                /* the sender by its most-derived address: it prints as its
                 * actual type, WsSessionSenderImpl, at that address
                 */
                obj.members()
                    .member_ref<UrlRouter const &>("url_router_", &r->url_router_)
                    .member_ref<rp<WsSender>>("sender_",
                                              dynamic_cast<void const *>(r->sender_.get()))
                    /* the server's, shared: printed in full in the server's members */
                    .member_ref<rp<PrintJson>>("pjson_", r->pjson_.get())
                    .member_as<std::unique_ptr<Json::CharReader>>("readjson_",
                                                                  std::string(r->readjson_ ? "set" : "null"))
                    .member_refs<std::vector<std::unique_ptr<WsSessionRouter::Subscription>>>("subscription_v_", sub_v)
                    .end();

                obj.close();
            }
        };

        void
        provide_router_json_printers(PrintJson * pjson)
        {
            pjson->provide_printer(Reflect::require<WsSessionRouter>(),
                                   std::make_unique<JsonPrinter_WsSessionRouter>());
            pjson->provide_printer(Reflect::require<WsSessionRouter::Subscription>(),
                                   std::make_unique<JsonPrinter_Subscription>());
        }

        std::size_t
        WsSessionRouter::n_subscription() const
        {
            std::lock_guard<std::mutex> lock(this->mutex_);

            std::size_t n = 0;
            for (auto const & sub : this->subscription_v_) {
                if (sub)
                    ++n;
            }

            return n;
        }
        void
        WsSessionRouter::Subscription::reflect_self(reflect::TypeDescrTable * /*table*/)
        {
            StructReflector<Subscription> sr;

            if (sr.is_incomplete()) {
                /* not endpoint_ or sink_: each printed in full elsewhere
                 * (the server's endpoints; the subscription's "sink") -- refs.
                 * Not callback_id_: CallbackId is not reflected
                 */
                REFLECT_MEMBER(sr, sub_id);
                REFLECT_MEMBER(sr, stream_name);
            }
        } /*reflect_self*/

        void
        WsSessionRouter::reflect_self(reflect::TypeDescrTable * table)
        {
            { StructReflector<WsSessionRouter> sr; }

            Subscription::reflect_self(table);
        } /*reflect_self*/
    } /*namespace web*/
} /*namespace xo*/

/* end WsSessionRouter.cpp */
