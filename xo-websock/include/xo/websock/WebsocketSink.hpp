/* file WebsocketSink.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

#include "WsSender.hpp"
#include <xo/printjson/PrintJson.hpp>
#include <xo/reflect/SelfTaggingDisplayable.hpp>
#include <cstdint>
#include <string>

namespace xo {
    namespace reflect { class TypeDescrTable; }
    namespace reflect { class TaggedPtr; }

    namespace web {
        /** @brief the outbound end of one websocket subscription.
         *
         *  The webserver creates one of these on behalf of an incoming
         *  subscription request.
         *  Matching  stream endpoint's subscribe function holds on to it
         *  to deliver events, using @ref notify_ev_tp.
         *  Each such event gets sent to remote connection as json.
         **/
        class WebsocketSink : public reflect::SelfTaggingDisplayable {
        public:
            /** describe WebsocketSink, and WebsocketSinkImpl, to xo-reflect.
             *  Called once, by websock_reflect_types() (websock_reflect.hpp).
             *  @p table is not used yet: xo-reflect's registration uses its
             *  process-wide table
             **/
            static void reflect_self(reflect::TypeDescrTable * table);

            using PrintJson = xo::json::PrintJson;
            using TaggedPtr = xo::reflect::TaggedPtr;

        public:
            /** sink handing each finished message to @p sender.
             *
             *  The webserver's session router makes one per subscription,
             *  with that session's sender.  A test sender lets the envelope
             *  -- stream, sub_id, seq, event -- be exercised without a live
             *  webserver.
             **/
            static rp<WebsocketSink> make(rp<WsSender> sender,
                                          rp<PrintJson> const & pjson,
                                          std::string const & stream_name,
                                          uint32_t sub_id);

            /** stream name from the subscription message that created this
             *  sink, i.e. the value of "stream" in
             *  @code {"cmd": "subscribe", "stream": "/this/stream/name"} @endcode
             **/
            virtual std::string const & stream_name() const = 0;

            /** lifetime count of events delivered to this sink.
             *  Also the "seq" of the NEXT outbound message: seq is 0-based
             *  and per subscription.
             **/
            virtual uint32_t n_in_ev() const = 0;

            /** render @p ev_tp as json and send it to this subscription's
             *  session, as
             *  @code
             *   {"stream": <name>,
             *    "sub_id": <id>,
             *    "seq": <n>,
             *    "event": <ev_tp as json>}
             *  @endcode
             *
             *  @c sub_id is the server-assigned id from the "subscribed" reply;
             *  it tells apart two subscriptions to one stream.
             *  @c seq values are consecutive, starting with 0.
             **/
            virtual void notify_ev_tp(TaggedPtr const & ev_tp) = 0;

            /** print this sink as json, within print @p state, for
             *  introspection (.xo-backlog/xo-websock/issues/10).  Default:
             *  id, refcount, stream.  The webserver's sink adds sub_id, seq,
             *  and its sender as a ref
             **/
            virtual void print_json(json::JsonPrintState & state) const;

            /* pretty(), display_string(): from ref::Displayable */
        }; /*WebsocketSink*/
    } /*namespace web*/
} /*namespace xo*/

/* end WebsocketSink.hpp */
