/* @file HttpEndpointDescr.cpp */

#include "HttpEndpointDescr.hpp"
#include <xo/ppsink/tag_ostream.hpp>   /* os << xtag(..) */
#include <xo/ppsink/tostr.hpp>

namespace xo {
    namespace web {
        HttpEndpointDescr::HttpEndpointDescr(std::string uri_pattern,
                                             HttpEndpointFn endpoint_fn)
            : uri_pattern_{std::move(uri_pattern)},
              endpoint_fn_{std::move(endpoint_fn)}
        {}

        void
        HttpEndpointDescr::display(std::ostream & os) const {
            /* function-local: a namespace-scope using here would become
             * ambiguous (not shadowing) if any header in this TU ever makes
             * legacy xo::xtag visible again.
             */
            using xo::pp::xtag;

            os << "<HttpEndpointDescr" << xtag("uri_pattern", uri_pattern_) << ">";
        } /*display*/

        std::string
        HttpEndpointDescr::display_string() const {
            using xo::pp::tostr;

            /* renders via operator<<(ostream&, HttpEndpointDescr) above --
             * ppsink has no Prettifier for this type, so pretty() falls through
             * to the operator<< path that tostr.hpp pulls in.
             */
            return tostr(*this);
        }
    } /*namespace web*/

} /*namespace xo*/


/* end HttpEndpointDescr.cpp */
