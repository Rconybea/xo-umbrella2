/** @file WsTestClient.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  A websocket client for live tests: libwebsockets' own client mode, on its
 *  own thread.  Test-only; see .xo-backlog/xo-websock/issues/09.
 **/

#pragma once

#include <libwebsockets.h>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace xo {
    namespace ut {
        /** @brief connects to ws://localhost:<port>/ with the webserver's
         *  "lws-minimal" protocol; sends text, and keeps every complete
         *  message received, in order.
         *
         *  Every wait_*() takes a timeout and reports whether its condition
         *  was met, so a test never sleeps for a guessed interval.
         *
         *  THREADING: public methods from the test's thread; the lws context
         *  lives on this client's own thread.
         **/
        class WsTestClient {
        public:
            using Timeout = std::chrono::milliseconds;

            /** connect to localhost:@p port; returns at once -- see
             *  wait_connected()
             **/
            explicit WsTestClient(std::int32_t port);
            /** disconnects, stops and joins the client thread **/
            ~WsTestClient();

            WsTestClient(WsTestClient const &) = delete;
            WsTestClient & operator=(WsTestClient const &) = delete;

            /** true once the websocket handshake has completed **/
            bool wait_connected(Timeout timeout);

            /** queue @p text to send as one websocket message **/
            void send(std::string text);

            /** true once at least @p n messages have been received **/
            bool wait_received(std::size_t n, Timeout timeout);

            /** copy of every message received so far, in order **/
            std::vector<std::string> received() const;

            /** close the connection from this end; true once closed **/
            bool close(Timeout timeout);

        private:
            static int callback(lws * wsi, lws_callback_reasons reason,
                                void * user, void * in, std::size_t len);

            void run();

        private:
            std::int32_t port_ = 0;

            /* protects everything below it */
            mutable std::mutex mutex_;
            std::condition_variable cv_;

            bool connected_ = false;
            bool closed_ = false;
            bool failed_ = false;
            bool close_requested_ = false;
            bool stop_ = false;

            /* waiting to be written */
            std::deque<std::string> outbox_;
            /* partial message, while fragments arrive */
            std::string partial_;
            /* complete messages received */
            std::vector<std::string> received_v_;

            /* service thread only */
            lws * wsi_ = nullptr;
            lws_context * cx_ = nullptr;

            std::thread thread_;
        }; /*WsTestClient*/
    } /*namespace ut*/
} /*namespace xo*/

/* end WsTestClient.hpp */
