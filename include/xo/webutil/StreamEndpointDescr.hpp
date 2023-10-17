/* file StreamEndpointDescr.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

#include "Alist.hpp"
#include "xo/refcnt/Refcounted.hpp"
#include "xo/callback/CallbackSet.hpp"
#include <functional>

namespace xo {
    namespace reactor { class AbstractSink; }

    namespace web {
        /* a function that creates an event subscription */
        using StreamSubscribeFn = std::function<fn::CallbackId (ref::rp<reactor::AbstractSink> const & ws_sink)>;
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

            void display(std::ostream & os) const;

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

        inline std::ostream &
        operator<<(std::ostream & os, StreamEndpointDescr const & x) {
            x.display(os);
            return os;
        } /*operator<<*/

    } /*namespace web*/
} /*namespace xo*/

/* end StreamEndpointDescr.hpp */
