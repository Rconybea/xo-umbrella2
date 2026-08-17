/** @file webutil_ostream.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  operator<<(std::ostream&, ..) for xo-webutil's public types.
 *
 *  The paved road to std::ostream.  xo-webutil's classes render into a
 *  xo::pp::PpSink -- see StreamEndpointDescr::pretty() / HttpEndpointDescr::pretty()
 *  and the Prettifier<> specializations beside them -- and their headers name no
 *  std::ostream at all.  Within xo, PpSink is the intended path: it composes
 *  (a descriptor nests inside an enclosing pretty_struct and participates in its
 *  line breaking), where an ostream inserter flattens.
 *
 *  But a PpSink is not what someone reaching for xo the first time already has
 *  in their hand -- they have a std::ostream.  So rather than making them
 *  discover FlatSink, this header gives them the inserter, implemented over
 *  FlatSink exactly as they would have written it:
 *
 *  @code
 *    #include <xo/webutil/webutil_ostream.hpp>
 *    std::cout << endpoint_descr << std::endl;
 *  @endcode
 *
 *  Equivalent, without this header:
 *  @code
 *    xo::pp::FlatSink sink(std::cout.rdbuf());
 *    endpoint_descr.pretty(sink);
 *  @endcode
 *  (NB two lines, not one: pretty() takes PpSink&, a non-const lvalue
 *  reference, so a FlatSink temporary will not bind.)
 *
 *  Expected usage inside xo: unit tests only.  New xo code that wants to print
 *  a descriptor should render into the PpSink it already has.
 *
 *  FlatSink renders splits as their flat spaces and never breaks a line, so
 *  output here is the single-line form regardless of length.
 **/

#pragma once

#include "HttpEndpointDescr.hpp"
#include "StreamEndpointDescr.hpp"
#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/pretty_ostream.hpp> /* xo::pp::pp_to_stream */
#include <xo/ppsink/pretty.hpp> /* PpSink::pp */
#include <ostream>

namespace xo {
    namespace web {
        inline std::ostream &
        operator<<(std::ostream & os, StreamEndpointDescr const & x) {
            return xo::pp::pp_to_stream(os, x);
        } /*operator<<*/

        inline std::ostream &
        operator<<(std::ostream & os, HttpEndpointDescr const & x) {
            return xo::pp::pp_to_stream(os, x);
        } /*operator<<*/
    } /*namespace web*/
} /*namespace xo*/

/* end webutil_ostream.hpp */
