/* file StreamEndpointDescr.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

#include "Alist.hpp"
#include <xo/refcnt/Refcounted.hpp>
#include <xo/callback/CallbackSet.hpp>
#include <xo/ppsink/PpSink.hpp>
#include <xo/ppsink/Prettifier.hpp>
#include <functional>

/* jsoncpp's parsed-value type, only named here: StreamReceiveFn takes one by
 * const reference, so a declaration is enough and xo-webutil needs no jsoncpp
 * dependency.  Code that builds or calls a receive function includes
 * <json/json.h> itself -- it reaches consumers through xo-websock.
 */
namespace Json { class Value; }

namespace xo {
    namespace web {
        /* the outbound end of one websocket subscription.  Defined in
         * xo-websock (xo/websock/WebsocketSink.hpp); only named here.
         */
        class WebsocketSink;

        /* a function that creates an event subscription */
        using StreamSubscribeFn = std::function<fn::CallbackId (rp<WebsocketSink> const & ws_sink)>;
        using StreamUnsubscribeFn = std::function<void (fn::CallbackId id)>;

        /* handle an application message sent by a subscriber on this stream,
         * i.e. the "msg" of
         *   {"cmd": "send", "stream": "/this/stream", "msg": <any JSON>}
         *
         * @p ws_sink is the SAME sink the subscribe function received for
         * that subscriber, so a reply sent through it reaches exactly the
         * session that asked.
         *
         * THREADING CONTRACT: invoked on the webserver's service thread, from
         * inside libwebsockets' receive callback.  A handler may send (e.g.
         * ws_sink->notify_ev_tp()) but must NOT block -- the whole server
         * stalls while it runs.  The flip side is useful: whatever the handler
         * mutates, and the frame it sends, happen on one thread.
         *
         * See .xo-backlog/xo-websock/issues/04.
         */
        using StreamReceiveFn = std::function<void (rp<WebsocketSink> const & ws_sink,
                                                    Json::Value const & msg)>;

        /* describes a stream endpoint
         * this comprises
         * - a uri pattern (matches stream name)
         * - a function that establishes subscription
         *   (by attaching supplied WebsocketSink to an event source)
         */
        class StreamEndpointDescr {
        public:
            using PpSink = xo::pp::PpSink;

        public:
            /* @p receive_fn optional: a stream without one rejects
             * {"cmd": "send", ...} with an error reply
             */
            StreamEndpointDescr(std::string uri_pattern,
                                StreamSubscribeFn subscribe_fn,
                                StreamUnsubscribeFn unsubscribe_fn,
                                StreamReceiveFn receive_fn = nullptr);

            std::string const & uri_pattern() const { return uri_pattern_; }
            StreamSubscribeFn const & subscribe_fn() const { return subscribe_fn_; }
            StreamUnsubscribeFn const & unsubscribe_fn() const { return unsubscribe_fn_; }
            StreamReceiveFn const & receive_fn() const { return receive_fn_; }

            /** structured pretty-printing: render this descriptor into @p sink.
             *
             *  See webutil_ostream.hpp for @c os << StreamEndpointDescr.
             **/
            void pretty(PpSink & sink) const;
            std::string display_string() const;

        private:
            /* unique pattern in URI-space for this endpoint
             * for example
             *    .uri_pattern = /stem/${foo}/${bar}
             * means this endpoint generates contents for uri's
             *    /stem/apple/banana
             *    /stem/aphid/green
             * but not for
             *    /stem/apple/banana/carrot
             */
            std::string uri_pattern_;
            /* a function that subscribes to an event stream
             * (by attaching a websocket sink)
             */
            StreamSubscribeFn subscribe_fn_;
            /* reverses effect of a particular call to .subscribe_fn */
            StreamUnsubscribeFn unsubscribe_fn_;
            /* handles {"cmd": "send", ...} from a subscriber; may be empty */
            StreamReceiveFn receive_fn_;
        }; /*StreamEndpointDescr*/

    } /*namespace web*/
} /*namespace xo*/

namespace xo::pp {
    /** pretty-print a StreamEndpointDescr into a PpSink. **/
    template <>
    struct Prettifier<xo::web::StreamEndpointDescr> {
        static void print(PpSink & sink, const xo::web::StreamEndpointDescr & x) {
            x.pretty(sink);
        }
    };
} /*namespace xo::pp*/

/* end StreamEndpointDescr.hpp */
