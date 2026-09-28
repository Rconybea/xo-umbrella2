/* file EndpointDescr.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <xo/refcnt/Refcounted.hpp>
#include <xo/ppsink/PpSink.hpp>
#include <xo/ppsink/Prettifier.hpp>
#include <functional>
#include <string>

namespace xo {
    namespace web {
        /** a function that answers an http request on demand **/
        using HttpHandler = std::function<HttpResponse (HttpRequest const &)>;

        /* describes an http endpoint --
         * this comprises:
         * - a uri pattern.
         * - a handler that answers requests for it
         */
        class HttpEndpointDescr {
        public:
            using PpSink = xo::pp::PpSink;

        public:
            HttpEndpointDescr(std::string uri_pattern,
                              HttpHandler handler);

            std::string const & uri_pattern() const { return uri_pattern_; }
            HttpHandler const & handler() const { return handler_; }

            /** structured pretty-printing: render this descriptor into @p sink.
             *
             *  This is the rendering primitive -- Prettifier below and
             *  display_string() both go through it.  Deliberately a PpSink
             *  rather than a std::ostream: see webutil_ostream.hpp if you want
             *  @c os << descr .
             **/
            void pretty(PpSink & sink) const;

            std::string display_string() const;

        private:
            /* unique pattern in URI-space for this endpoint.
             * for example
             *    .uri_pattern = /stem/${foo}/${bar}
             * means this endpoint generates contents for uri's
             *    /stem/apple/banana
             *    /stem/aphid/green
             * but not for
             *    /stem/apple/banana/carrot
             * A variable matches one path segment; a LAST variable written
             * ${name...} matches the rest of the uri, slashes included:
             *    .uri_pattern = /src/${path...}
             * matches /src/a.hpp and /src/xo-foo/include/b.hpp
             */
            std::string uri_pattern_;
            /* answers a request that matches .uri_pattern:
             *   .handler(request) -> response
             * request.vars() holds the value of each variable in
             * .uri_pattern (surrounded by ${..})
             */
            HttpHandler handler_;
        }; /*HttpEndpointDescr*/

    } /*namespace web*/
} /*namespace xo*/

namespace xo::pp {
    /** pretty-print an HttpEndpointDescr into a PpSink.
     *
     *  Lives here, not in a separate _pp.hpp, because the class already
     *  declares pretty(PpSink&) -- so this header depends on xo-ppsink either
     *  way, and making the ppsink path the opt-in one would get the ergonomics
     *  backwards.  webutil_ostream.hpp is the opt-in header, for the ostream
     *  path we would rather callers inside xo did not take.
     **/
    template <>
    struct Prettifier<xo::web::HttpEndpointDescr> {
        static void print(PpSink & sink, const xo::web::HttpEndpointDescr & x) {
            x.pretty(sink);
        }
    };
} /*namespace xo::pp*/

/* end EndpointDescr.hpp */
