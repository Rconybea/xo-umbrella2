/** @file WsSessionRouter.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "WebsocketSink.hpp"
#include <xo/refcnt/Refcounted.hpp>
#include <xo/callback/CallbackId.hpp>
#include <cstddef>
#include <cstdint>
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
        class UrlRouter;

        /** @brief one websocket session's subscriptions, and the commands
         *  that manage and use them.
         *
         *  Handles each inbound message a session sends, and answers:
         *
         *  @code
         *  -> {"cmd": "subscribe",   "stream": "/x"}
         *  <- {"cmd": "subscribed",  "stream": "/x", "sub_id": 0}
         *  <- {"stream": "/x", "sub_id": 0, "seq": 0, "event": ...}   (frames)
         *  -> {"cmd": "send",        "sub_id": 0, "msg": <any JSON>}
         *  -> {"cmd": "unsubscribe", "sub_id": 0}
         *  <- {"cmd": "unsubscribed", "sub_id": 0}
         *  @endcode
         *
         *  - subscribe: find the stream's endpoint, assign a sub_id, make a
         *    sink carrying it, answer "subscribed" -- BEFORE the endpoint's
         *    subscribe function runs, since that may send an initial frame and
         *    the client must learn its sub_id first -- then subscribe.
         *  - send: run the subscription's endpoint's receive function with the
         *    parsed msg and that subscription's sink, so a reply reaches
         *    exactly this session.
         *  - unsubscribe: run the endpoint's unsubscribe, retire the sub_id.
         *
         *  Every failure gets an error reply naming what it can:
         *  @code {"error": <reason>, "stream": <name>, "sub_id": <n>} @endcode
         *
         *  sub_ids are assigned by the server: a subscription's index in
         *  .subscription_v.  Unsubscribing leaves the slot EMPTY and it is never
         *  reused, so a stale sub_id is an error and can never reach a
         *  different subscription.  See .xo-backlog/xo-websock/issues/06.
         *
         *  Knows nothing about libwebsockets.  It reaches the server only
         *  through the server's UrlRouter (to find endpoints) and two injected
         *  functions -- make a sink, send text to this session -- which is
         *  what lets it be unit-tested without a socket.  Split out of
         *  WebserverImpl on 2026-09-26; see .xo-backlog/xo-websock/issues/04.
         *
         *  Threading: perform_cmd() is called on the webserver's service
         *  thread.  No internal lock is held while an endpoint's subscribe,
         *  unsubscribe or receive function runs, since any of them may send
         *  -- which re-enters the server.
         **/
        class WsSessionRouter {
        public:
            /** new sink delivering to this session, for @p stream_name, whose
             *  envelopes carry @p sub_id
             **/
            using SinkFactory = std::function<rp<WebsocketSink> (std::string const & stream_name,
                                                                 std::uint32_t sub_id)>;
            /** send @p text to this session, outside any subscription **/
            using ReplyFn = std::function<void (std::string text)>;

        public:
            /** @p url_router is borrowed: it must outlive this router.
             *  Endpoints are server-wide and outlive every session.
             **/
            WsSessionRouter(UrlRouter const & url_router,
                            SinkFactory sink_fn,
                            ReplyFn reply_fn);
            ~WsSessionRouter();

            WsSessionRouter(WsSessionRouter const &) = delete;
            WsSessionRouter & operator=(WsSessionRouter const &) = delete;

            /** handle one inbound message from this session **/
            void perform_cmd(std::string_view incoming_cmd);

            /** unsubscribe everything; the session is closing **/
            void unsubscribe_all();

            /** number of ACTIVE subscriptions -- unsubscribed slots excluded **/
            std::size_t n_subscription() const;

        private:
            struct Subscription;

            void subscribe(std::string const & stream_name);
            void send(std::uint32_t sub_id, Json::Value const & msg);
            void unsubscribe(std::uint32_t sub_id);

            /** reads "sub_id" from @p root; replies with an error and returns
             *  false if absent or not a non-negative integer
             **/
            bool require_sub_id(Json::Value const & root,
                                char const * cmd,
                                std::uint32_t * p_sub_id);

            /** active subscription for @p sub_id, copied out under the lock;
             *  replies with an error and returns false if unknown or already
             *  unsubscribed
             **/
            bool lookup_active(std::uint32_t sub_id, Subscription * p_copy);

            /** send @p msg, as json text, to this session **/
            void reply(Json::Value const & msg);
            /** error reply; @p stream_name and @p sub_id included when given **/
            void reply_error(std::string const & reason,
                             std::string const * stream_name = nullptr,
                             std::uint32_t const * sub_id = nullptr);

        private:
            /* finds the endpoint serving a stream name.  Borrowed; see ctor */
            UrlRouter const & url_router_;
            SinkFactory sink_fn_;
            ReplyFn reply_fn_;

            /* one per session: jsoncpp readers are not threadsafe */
            std::unique_ptr<Json::CharReader> readjson_;

            /* protects .subscription_v */
            mutable std::mutex mutex_;
            /* this session's subscriptions; index IS the sub_id.  An
             * unsubscribed slot is null, and never reused
             */
            std::vector<std::unique_ptr<Subscription>> subscription_v_;
        }; /*WsSessionRouter*/
    } /*namespace web*/
} /*namespace xo*/

/* end WsSessionRouter.hpp */
