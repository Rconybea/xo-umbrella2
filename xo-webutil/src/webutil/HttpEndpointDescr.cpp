/* @file HttpEndpointDescr.cpp */

#include "HttpEndpointDescr.hpp"
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/pretty_struct.hpp>
#include <xo/ppsink/quoted.hpp>

namespace xo {
    namespace web {
        HttpEndpointDescr::HttpEndpointDescr(std::string uri_pattern,
                                             HttpHandler handler)
            : uri_pattern_{std::move(uri_pattern)},
              handler_{std::move(handler)}
        {}

        void
        HttpEndpointDescr::pretty(xo::pp::PpSink & sink) const {
            using xo::pp::field;
            using xo::pp::unq;

            /* unq(): quotes only when bare would be ambiguous, so an ordinary
             * uri pattern renders exactly as before, while one containing
             * whitespace -- or an empty one, which used to render as the
             * misleading "<HttpEndpointDescr :uri_pattern >" -- gets quoted.
             */
            sink.pretty_struct("HttpEndpointDescr",
                               field("uri_pattern", unq(uri_pattern_)));
        } /*pretty*/

        std::string
        HttpEndpointDescr::display_string() const {
            using xo::pp::tostr;

            /* routes through Prettifier<HttpEndpointDescr> (declared in the
             * header), which calls pretty() above.  No operator<< involved.
             */
            return tostr(*this);
        }
    } /*namespace web*/

} /*namespace xo*/


/* end HttpEndpointDescr.cpp */
