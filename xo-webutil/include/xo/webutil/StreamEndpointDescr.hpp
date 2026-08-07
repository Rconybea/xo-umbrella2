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

namespace xo {
    namespace reactor { class AbstractSink; }

    namespace web {
        /* a function that creates an event subscription */
        using StreamSubscribeFn = std::function<fn::CallbackId (rp<reactor::AbstractSink> const & ws_sink)>;
        using StreamUnsubscribeFn = std::function<void (fn::CallbackId id)>;

        /* describes a stream endpoint
         * this comprises
         * - a uri pattern (matches stream name)
         * - a function that establishes subscription
         *   (by attaching supplied WebsocketSink to an event source)
         */
        class StreamEndpointDescr {
        public:
            StreamEndpointDescr(std::string uri_pattern,
                                StreamSubscribeFn subscribe_fn,
                                StreamUnsubscribeFn unsubscribe_fn);

            std::string const & uri_pattern() const { return uri_pattern_; }
            StreamSubscribeFn const & subscribe_fn() const { return subscribe_fn_; }
            StreamUnsubscribeFn const & unsubscribe_fn() const { return unsubscribe_fn_; }

            /** structured pretty-printing: render this descriptor into @p sink.
             *
             *  This is the rendering primitive -- Prettifier below and
             *  display_string() both go through it.  Deliberately a PpSink
             *  rather than a std::ostream: see webutil_ostream.hpp if you want
             *  @c os << descr .
             **/
            void pretty(xo::pp::PpSink & sink) const;

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
        }; /*StreamEndpointDescr*/

    } /*namespace web*/
} /*namespace xo*/

namespace xo::pp {
    /** pretty-print a StreamEndpointDescr into a PpSink.
     *
     *  Lives here, not in a separate _pp.hpp, because the class already
     *  declares pretty(PpSink&) -- so this header depends on xo-ppsink either
     *  way, and making the ppsink path the opt-in one would get the ergonomics
     *  backwards.  webutil_ostream.hpp is the opt-in header, for the ostream
     *  path we would rather callers inside xo did not take.
     **/
    template <>
    struct Prettifier<xo::web::StreamEndpointDescr> {
        static void print(PpSink & sink, const xo::web::StreamEndpointDescr & x) {
            x.pretty(sink);
        }
    };
} /*namespace xo::pp*/

/* end StreamEndpointDescr.hpp */
