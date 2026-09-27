/** @file WsSender.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include <xo/refcnt/Refcounted.hpp>
#include <string>

namespace xo {
    namespace web {
        /** @brief delivers finished outbound text to ONE websocket session.
         *
         *  The layer below WebsocketSink: a sink turns an event into json
         *  (envelope, sub_id, seq); a sender puts the finished text on the
         *  wire.  Replaced WebsocketSink::SendFn, a std::function; see
         *  .xo-backlog/xo-websock/issues/05.
         *
         *  THREADING: send_text() may be called from any thread, and must
         *  not block.
         **/
        class WsSender : public ref::Refcount {
        public:
            /** send @p text as one complete websocket message.
             *  After the session closes, dropped rather than delivered.
             **/
            virtual void send_text(std::string text) = 0;

            /** false once the destination session has closed **/
            virtual bool is_open() const = 0;
        }; /*WsSender*/
    } /*namespace web*/
} /*namespace xo*/

/* end WsSender.hpp */
