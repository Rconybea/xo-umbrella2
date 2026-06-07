/* @file HttpEndpointDescr.cpp */

#include "HttpEndpointDescr.hpp"
#include "xo/indentlog/print/tag.hpp"
#include "xo/indentlog/print/tostr.hpp"

namespace xo {
    namespace web {
        HttpEndpointDescr::HttpEndpointDescr(std::string uri_pattern,
                                             HttpEndpointFn endpoint_fn)
            : uri_pattern_{std::move(uri_pattern)},
              endpoint_fn_{std::move(endpoint_fn)}
        {}

        void
        HttpEndpointDescr::display(std::ostream & os) const {
            os << "<HttpEndpointDescr" << xtag("uri_pattern", uri_pattern_) << ">";
        } /*display*/

        std::string
        HttpEndpointDescr::display_string() const { return tostr(*this); }
    } /*namespace web*/

} /*namespace xo*/


/* end HttpEndpointDescr.cpp */
