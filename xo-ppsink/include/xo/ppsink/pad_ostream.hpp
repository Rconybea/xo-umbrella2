/** @file pad_ostream.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  operator<<(std::ostream&, pad_impl): render a pad()/spaces() to an ostream.
 *
 *  A migration bridge, mirroring tag_ostream.hpp / quoted_ostream.hpp: the
 *  legacy `os << pad(indent)` idiom appears in hand-indented ostream printers
 *  (xo-ordinaltree's tree dumps), and this lets such a site move to
 *  xo::pp::pad without being rewritten.  Routes through a FlatSink wrapping
 *  @p os, reusing Prettifier<pad_impl>.
 *
 *  Segregated from pad.hpp, which stays free of <ostream>.
 **/

#pragma once

#include "FlatSink.hpp"
#include "pad.hpp"
#include "pretty.hpp" /* PpSink::pp */

namespace xo::pp {
    inline std::ostream &
    operator<<(std::ostream & os, pad_impl x) {
        FlatSink sink(os);

        sink.pp(x);

        return os;
    }
} /*namespace xo::pp*/

/* end pad_ostream.hpp */
