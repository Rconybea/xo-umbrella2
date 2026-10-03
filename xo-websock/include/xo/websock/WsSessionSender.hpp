/** @file WsSessionSender.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "WsSender.hpp"
#include <atomic>
#include <cstdint>
#include <string>

namespace xo {
    namespace web {
        /** @brief sends to one websocket session of a webserver: its router's
         *  replies, and frames from every sink that router makes.
         *
         *  One per session, created at session open.  Forwards to
         *  @c target->send_text(session_id, text); in production @p Target is
         *  WebserverImpl.  A template so it can be unit-tested with a fake
         *  target -- WebserverImpl exists only inside Webserver.cpp.
         *
         *  Holds a plain @p Target pointer, not rp<>: the server owns its
         *  sessions, so an rp<> here would be a cycle.
         *
         *  close() at session close (and, as a backstop, when the server is
         *  destroyed): from then on send_text() drops what it is given, so a
         *  sink the application keeps past its session never reaches the
         *  server again -- which may by then be freed.  See
         *  .xo-backlog/xo-websock/issues/05 and 08.
         *
         *  THREADING: send_text() and is_open() from any thread.  close()
         *  racing a send_text() on another thread may let that one send
         *  through; the target must tolerate a send to a closed session (the
         *  webserver drops it: ids are never reused).
         **/
        template <typename Target>
        class WsSessionSender : public WsSender {
        public:
            WsSessionSender(Target * target, std::uint64_t session_id)
                : target_{target}, session_id_{session_id} {}

            std::uint64_t session_id() const { return session_id_; }

            void send_text(std::string text) override {
                /* a closed sender may outlive its target: check before use */
                if (this->open_.load())
                    this->target_->send_text(this->session_id_, std::move(text));
            }

            bool is_open() const override { return open_.load(); }

            /** stop delivering; idempotent **/
            void close() { open_.store(false); }

        private:
            /* reads private members, for "_members_" (Webserver.cpp) */
            friend class JsonPrinter_WsSessionSender;

        private:
            /* where text goes; borrowed, see class comment */
            Target * target_ = nullptr;
            /* the session this sender delivers to */
            std::uint64_t session_id_ = 0;
            /* false once closed; never reopens */
            std::atomic<bool> open_{true};
        }; /*WsSessionSender*/
    } /*namespace web*/
} /*namespace xo*/

/* end WsSessionSender.hpp */
