/** @file StreamReceiver.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include <xo/refcnt/Refcounted.hpp>

/* jsoncpp's parsed-value type, only named here: receive() takes one by const
 * reference, so a declaration is enough and xo-webutil needs no jsoncpp
 * dependency.  Code that implements or calls a receiver includes
 * <json/json.h> itself -- it reaches consumers through xo-websock.
 */
namespace Json { class Value; }

namespace xo {
    namespace web {
        /* the outbound end of one websocket subscription.  Defined in
         * xo-websock (xo/websock/WebsocketSink.hpp); only named here.
         */
        class WebsocketSink;

        /** @brief handles application messages sent by a stream's subscribers.
         *
         *  Receives the "msg" of
         *  @code {"cmd": "send", "sub_id": <n>, "msg": <any JSON>} @endcode
         *
         *  Optional part of a stream endpoint (see StreamEndpointDescr): a
         *  stream without a receiver rejects "send" with an error reply.
         *
         *  Application code implements this -- e.g. the flywheel demo's "step"
         *  handler.  Replaced a std::function, StreamReceiveFn; see
         *  .xo-backlog/xo-websock/issues/05.
         *
         *  THREADING CONTRACT: receive() is invoked on the webserver's service
         *  thread, from inside libwebsockets' receive callback.  It may send
         *  (e.g. ws_sink->notify_ev_tp()) but must NOT block -- the whole
         *  server stalls while it runs.  The flip side is useful: whatever it
         *  mutates, and the frame it sends, happen on one thread.
         **/
        class StreamReceiver : public ref::Refcount {
        public:
            /** handle @p msg from one subscriber.
             *
             *  @p ws_sink is the SAME sink the stream's subscribe function
             *  received for that subscription, so a reply sent through it
             *  reaches exactly the session that asked.
             *
             *  An exception is caught by the webserver and becomes an error
             *  reply to that session.
             **/
            virtual void receive(rp<WebsocketSink> const & ws_sink,
                                 Json::Value const & msg) = 0;
        }; /*StreamReceiver*/
    } /*namespace web*/
} /*namespace xo*/

/* end StreamReceiver.hpp */
