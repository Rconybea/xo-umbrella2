/* file WebsocketSink.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

#include <xo/printjson/PrintJson.hpp>
#include <xo/refcnt/Displayable.hpp>
#include <cstdint>
#include <string>

namespace xo {
    namespace reflect { class TaggedPtr; }

    namespace web {
        class Webserver;

        /** @brief the outbound end of one websocket subscription.
         *
         *  The webserver creates one of these on behalf of an incoming
         *  subscription request.
         *  Matching  stream endpoint's subscribe function holds on to it
         *  to deliver events, using @ref notify_ev_tp.
         *  Each such event gets sent to remote connection as json.
         **/
        class WebsocketSink : public ref::Displayable {
        public:
            using PrintJson = xo::json::PrintJson;
            using TaggedPtr = xo::reflect::TaggedPtr;

        public:
            static rp<WebsocketSink> make(rp<Webserver> const & websrv,
                                          rp<PrintJson> const & pjson,
                                          uint32_t session_id,
                                          std::string const & stream_name);

            /** stream name from the subscription message that created this
             *  sink, i.e. the value of "stream" in
             *  @code {"cmd": "subscribe", "stream": "/this/stream/name"} @endcode
             **/
            virtual std::string const & stream_name() const = 0;

            /** lifetime count of events delivered to this sink **/
            virtual uint32_t n_in_ev() const = 0;

            /** render @p ev_tp as json and send it to this subscription's
             *  session
             **/
            virtual void notify_ev_tp(TaggedPtr const & ev_tp) = 0;

            /* pretty(), display_string(): from ref::Displayable */
        }; /*WebsocketSink*/
    } /*namespace web*/
} /*namespace xo*/

/* end WebsocketSink.hpp */
