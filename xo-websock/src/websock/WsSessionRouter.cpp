/** @file WsSessionRouter.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "WsSessionRouter.hpp"
#include "UrlRouter.hpp"
#include "DynamicEndpoint.hpp"
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag_ostream.hpp>      /* xtag(..) */
#include <json/json.h>
#include <exception>

namespace xo {
    using xo::fn::CallbackId;

    namespace web {
        using xo::pp::scope;
        using xo::pp::xtag;

        struct WsSessionRouter::Subscription {
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
                                         SinkFactory sink_fn,
                                         ReplyFn reply_fn)
            : url_router_{url_router},
              sink_fn_{std::move(sink_fn)},
              reply_fn_{std::move(reply_fn)},
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
            rp<WebsocketSink> sink = sink_fn_(stream_name, sub_id);

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

            this->reply_fn_(Json::writeString(wb, msg));
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
    } /*namespace web*/
} /*namespace xo*/

/* end WsSessionRouter.cpp */
