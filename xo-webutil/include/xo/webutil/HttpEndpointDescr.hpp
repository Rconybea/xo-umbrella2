/* file EndpointDescr.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

#include "Alist.hpp"
#include <xo/refcnt/Refcounted.hpp>
#include <xo/ppsink/PpSink.hpp>
#include <xo/ppsink/Prettifier.hpp>
#include <functional>
/* HttpEndpointFn names std::ostream* -- a pointer, so a forward declaration is
 * enough.  Was arriving transitively (via Refcounted.hpp's operator<<), leaving
 * this header not self-contained.
 */
#include <iosfwd>
#include <string>

namespace xo {
    namespace web {
        /* a function that can deliver http content on demand. */
        using HttpEndpointFn = std::function<void (std::string const &,
                                                   Alist const &,
                                                   std::ostream *)>;

        /* describes an http endpoint --
         * this comprises:
         * - a uri pattern.
         * - a function that can deliver http content on demand
         */
        class HttpEndpointDescr {
        public:
            using PpSink = xo::pp::PpSink;

        public:
            HttpEndpointDescr(std::string uri_pattern,
                              HttpEndpointFn endpoint_fn);

            std::string const & uri_pattern() const { return uri_pattern_; }
            HttpEndpointFn const & endpoint_fn() const { return endpoint_fn_; }

            /** structured pretty-printing: render this descriptor into @p sink.
             *
             *  This is the rendering primitive -- Prettifier below and
             *  display_string() both go through it.  Deliberately a PpSink
             *  rather than a std::ostream: see webutil_ostream.hpp if you want
             *  @c os << descr .
             *
             *  NB the std::ostream* in HttpEndpointFn above is a different
             *  thing -- it carries HTTP response payload, not diagnostics.
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
             */
            std::string uri_pattern_;
            /* a function that can construct http output on demand
             *   .endpoint_fn(uri, alist, &os)
             * writes http output to os.   output is parameterized
             * by name-value pairs in alist,  and is prepared on behalf
             * of .uri_pattern
             * alist will report name-value pairs for each variable that
             * appears in .uri_pattern (surrounded by ${..})
             */
            HttpEndpointFn endpoint_fn_;
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
