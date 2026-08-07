/* @file StreamEndpointDescr.cpp */

#include "StreamEndpointDescr.hpp"
#include <xo/ppsink/pretty_struct.hpp>
#include <xo/ppsink/quoted.hpp>
#include <xo/ppsink/tostr.hpp>

namespace xo {
    namespace web {
        StreamEndpointDescr::StreamEndpointDescr(std::string uri_pattern,
                                                 StreamSubscribeFn subscribe_fn,
                                                 StreamUnsubscribeFn unsubscribe_fn)
            : uri_pattern_{std::move(uri_pattern)},
              subscribe_fn_{std::move(subscribe_fn)},
              unsubscribe_fn_{std::move(unsubscribe_fn)}
        {}

        void
        StreamEndpointDescr::pretty(xo::pp::PpSink & sink) const {
            using xo::pp::field;
            using xo::pp::unq;

            /* unq(): quotes only when bare would be ambiguous, so an ordinary
             * uri pattern renders exactly as before, while one containing
             * whitespace -- or an empty one, which used to render as the
             * misleading "<StreamEndpointDescr :uri_pattern >" -- gets quoted.
             */
            sink.pretty_struct("StreamEndpointDescr",
                               field("uri_pattern", unq(uri_pattern_)));
        } /*pretty*/

        std::string
        StreamEndpointDescr::display_string() const {
            using xo::pp::tostr;

            /* routes through Prettifier<StreamEndpointDescr> (declared in the
             * header), which calls pretty() above.  No operator<< involved.
             */
            return tostr(*this);
        }
    } /*namespace web*/
} /*namespace xo*/


/* end StreamEndpointDescr.cpp */
