/** @file WsSessionRouter.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "WsSessionRouter.hpp"
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
            /* stream name from the subscribe command; what "send" matches */
            std::string stream_name_;
            /* endpoint serving the stream.  Owned by the webserver's stream
             * map, which outlives every session
             */
            DynamicEndpoint * endpoint_ = nullptr;
            /* id from the endpoint's subscribe function, for unsubscribe */
            CallbackId callback_id_;
            /* sink delivering to this session for this stream */
            rp<WebsocketSink> sink_;
        };

        WsSessionRouter::WsSessionRouter(EndpointLookup lookup_fn,
                                         SinkFactory sink_fn,
                                         ReplyFn reply_fn)
            : lookup_fn_{std::move(lookup_fn)},
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
                this->reply_error(nullptr, "malformed json: " + err);
                return;
            }

            /* Type checks BEFORE any field access.  jsoncpp throws
             * Json::LogicError from operator[] on a non-object and from
             * asString() on a non-string, and an exception escaping into
             * libwebsockets' C callback terminates the server.  Before this
             * class, a client sending a bare json string or array did exactly
             * that.
             */
            if (!root.isObject()) {
                this->reply_error(nullptr, "message is not a json object");
                return;
            }

            Json::Value const & cmd_v = root["cmd"];
            Json::Value const & stream_v = root["stream"];

            std::string cmd = (cmd_v.isString() ? cmd_v.asString() : std::string());
            std::string stream_name = (stream_v.isString() ? stream_v.asString() : std::string());

            log && log(xtag("cmd", cmd), xtag("stream", stream_name));

            if (cmd == "subscribe") {
                this->subscribe(stream_name);
            } else if (cmd == "send") {
                if (stream_name.empty()) {
                    this->reply_error(nullptr, "send requires a \"stream\"");
                    return;
                }

                /* msg is optional; absent reads as json null */
                this->send(stream_name, root["msg"]);
            } else {
                /* unknown command: logged and ignored, as before.  Unlike a
                 * failed send this gets no error reply -- a deliberate scope
                 * limit, see .xo-backlog/xo-websock/issues/04
                 */
                log && log("unknown command, ignored");
            }
        } /*perform_cmd*/

        void
        WsSessionRouter::subscribe(std::string const & stream_name)
        {
            scope log(XO_ENTER0_(info), xtag("stream", stream_name));

            DynamicEndpoint * endpoint = lookup_fn_(stream_name);

            if (!endpoint) {
                /* unchanged behaviour: an unmatched subscribe is silent */
                log && log("endpoint not found");
                return;
            }

            rp<WebsocketSink> sink = sink_fn_(stream_name);

            std::unique_ptr<Subscription> sub(new Subscription());
            sub->stream_name_ = stream_name;
            sub->endpoint_ = endpoint;
            sub->sink_ = sink;

            Subscription * sub_addr = sub.get();

            {
                std::lock_guard<std::mutex> lock(this->mutex_);
                this->subscription_v_.push_back(std::move(sub));
            }

            /* lock dropped: subscribe may send, e.g. an initial frame */
            CallbackId id = endpoint->subscribe(stream_name, sink);

            {
                std::lock_guard<std::mutex> lock(this->mutex_);
                sub_addr->callback_id_ = id;
            }
        } /*subscribe*/

        void
        WsSessionRouter::send(std::string const & stream_name,
                              Json::Value const & msg)
        {
            DynamicEndpoint * endpoint = nullptr;
            rp<WebsocketSink> sink;

            {
                std::lock_guard<std::mutex> lock(this->mutex_);

                /* first match: subscribing twice to one stream is allowed, and
                 * messages go to the earlier subscription
                 */
                for (auto const & sub : this->subscription_v_) {
                    if (sub->stream_name_ == stream_name) {
                        endpoint = sub->endpoint_;
                        sink = sub->sink_;
                        break;
                    }
                }
            }

            if (!endpoint) {
                this->reply_error(&stream_name, "not subscribed to stream");
                return;
            }

            if (!endpoint->has_receive()) {
                this->reply_error(&stream_name, "stream does not accept messages");
                return;
            }

            /* lock dropped: the handler is expected to send */
            try {
                endpoint->receive(sink, msg);
            } catch (std::exception & ex) {
                /* never let a handler's exception reach libwebsockets */
                this->reply_error(&stream_name,
                                  std::string("stream handler failed: ") + ex.what());
            }
        } /*send*/

        void
        WsSessionRouter::reply_error(std::string const * stream_name,
                                     std::string const & reason)
        {
            Json::Value reply(Json::objectValue);

            reply["error"] = reason;
            if (stream_name)
                reply["stream"] = *stream_name;

            /* jsoncpp for the writing too: reason may carry arbitrary text
             * (a parser message, an exception's what()) that must be escaped
             */
            Json::StreamWriterBuilder wb;
            wb["indentation"] = "";

            this->reply_fn_(Json::writeString(wb, reply));
        } /*reply_error*/

        void
        WsSessionRouter::unsubscribe_all()
        {
            std::vector<std::unique_ptr<Subscription>> subs;

            {
                std::lock_guard<std::mutex> lock(this->mutex_);
                subs.swap(this->subscription_v_);
            }

            /* lock dropped, as for subscribe */
            for (auto const & sub : subs)
                sub->endpoint_->unsubscribe(sub->callback_id_);
        } /*unsubscribe_all*/

        std::size_t
        WsSessionRouter::n_subscription() const
        {
            std::lock_guard<std::mutex> lock(this->mutex_);

            return this->subscription_v_.size();
        }
    } /*namespace web*/
} /*namespace xo*/

/* end WsSessionRouter.cpp */
