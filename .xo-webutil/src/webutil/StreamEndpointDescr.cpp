/* @file StreamEndpointDescr.cpp */

#include "StreamEndpointDescr.hpp"
#include "xo/indentlog/print/tag.hpp"
#include "xo/indentlog/print/tostr.hpp"

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
            os << "<StreamEndpointDescr" << xtag("uri_pattern", uri_pattern_) << ">";
        } /*display*/

        std::string
        StreamEndpointDescr::display_string() const { return tostr(*this); }
    } /*namespace web*/
} /*namespace xo*/


/* end StreamEndpointDescr.cpp */
