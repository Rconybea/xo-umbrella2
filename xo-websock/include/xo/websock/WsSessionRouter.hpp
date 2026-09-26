/** @file WsSessionRouter.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "WebsocketSink.hpp"
#include <xo/refcnt/Refcounted.hpp>
#include <xo/callback/CallbackId.hpp>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace Json {
    class CharReader;
    class Value;
}

namespace xo {
    namespace web {
        class DynamicEndpoint;

        /** @brief one websocket session's subscriptions, and the commands
         *  that manage and use them.
         *
         *  Handles each inbound message a session sends:
         *
         *  @code
         *  {"cmd": "subscribe", "stream": "/x"}
         *  {"cmd": "send",      "stream": "/x", "msg": <any JSON>}
         *  @endcode
         *
         *  - subscribe: find the stream's endpoint, make a sink for it, and
         *    run the endpoint's subscribe function.  An unmatched stream is
         *    logged and otherwise ignored, as before this class existed.
         *  - send: find THIS session's subscription to the stream and run its
         *    endpoint's receive function with the parsed msg and that
         *    subscription's sink, so a reply reaches exactly this session.
         *    Every failure on this path gets an error reply:
         *    @code {"error": <reason>, "stream": <name>} @endcode
         *    with "stream" absent when there is none to name.
         *
         *  Knows nothing about libwebsockets.  It reaches the server only
         *  through three injected functions -- look up an endpoint, make a
         *  sink, send text to this session -- which is what lets it be
         *  unit-tested with fakes.  Split out of WebserverImpl on 2026-09-26;
         *  see .xo-backlog/xo-websock/issues/04.
         *
         *  Threading: perform_cmd() is called on the webserver's service
         *  thread.  No internal lock is held while an endpoint's subscribe,
         *  unsubscribe or receive function runs, since any of them may send
         *  -- which re-enters the server.
         **/
        class WsSessionRouter {
        public:
            /** endpoint serving @p stream_name, or nullptr **/
            using EndpointLookup = std::function<DynamicEndpoint * (std::string const & stream_name)>;
            /** new sink delivering to this session, for @p stream_name **/
            using SinkFactory = std::function<rp<WebsocketSink> (std::string const & stream_name)>;
            /** send @p text to this session, outside any subscription **/
            using ReplyFn = std::function<void (std::string text)>;

        public:
            WsSessionRouter(EndpointLookup lookup_fn,
                            SinkFactory sink_fn,
                            ReplyFn reply_fn);
            ~WsSessionRouter();

            WsSessionRouter(WsSessionRouter const &) = delete;
            WsSessionRouter & operator=(WsSessionRouter const &) = delete;

            /** handle one inbound message from this session **/
            void perform_cmd(std::string_view incoming_cmd);

            /** unsubscribe everything; the session is closing **/
            void unsubscribe_all();

            /** number of active subscriptions **/
            std::size_t n_subscription() const;

        private:
            struct Subscription;

            void subscribe(std::string const & stream_name);
            void send(std::string const & stream_name, Json::Value const & msg);
            /** error reply; @p stream_name may be nullptr **/
            void reply_error(std::string const * stream_name, std::string const & reason);

        private:
            EndpointLookup lookup_fn_;
            SinkFactory sink_fn_;
            ReplyFn reply_fn_;

            /* one per session: jsoncpp readers are not threadsafe */
            std::unique_ptr<Json::CharReader> readjson_;

            /* protects .subscription_v */
            mutable std::mutex mutex_;
            /* this session's subscriptions, in arrival order */
            std::vector<std::unique_ptr<Subscription>> subscription_v_;
        }; /*WsSessionRouter*/
    } /*namespace web*/
} /*namespace xo*/

/* end WsSessionRouter.hpp */
