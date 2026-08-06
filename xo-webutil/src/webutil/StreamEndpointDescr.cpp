/* @file StreamEndpointDescr.cpp */

#include "StreamEndpointDescr.hpp"
#include <xo/ppsink/tag_ostream.hpp>   /* os << xtag(..) */
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
        StreamEndpointDescr::display(std::ostream & os) const {
            /* function-local: a namespace-scope using here would become
             * ambiguous (not shadowing) if any header in this TU ever makes
             * legacy xo::xtag visible again.
             */
            using xo::pp::xtag;

            os << "<StreamEndpointDescr" << xtag("uri_pattern", uri_pattern_) << ">";
        } /*display*/

        std::string
        StreamEndpointDescr::display_string() const {
            using xo::pp::tostr;

            /* renders via operator<<(ostream&, StreamEndpointDescr) above --
             * ppsink has no Prettifier for this type, so pretty() falls through
             * to the operator<< path that tostr.hpp pulls in.
             */
            return tostr(*this);
        }
    } /*namespace web*/
} /*namespace xo*/


/* end StreamEndpointDescr.cpp */
