/** @file pretty_ostream.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  pretty() together with the <ostream> its operator<< fallback needs.
 *
 *  Anyone calling pretty() (in particular a Prettifier implementor that
 *  renders members via pretty()) includes one of:
 *   - pretty.hpp          -- when every value reaches a Prettifier<T> or a
 *                            string-like leaf, so the operator<< fallback is
 *                            never instantiated (ostream-free), or
 *   - pretty_ostream.hpp  -- when some value is an opaque leaf rendered via
 *                            operator<<, which needs <ostream> visible at the
 *                            point pretty<T> is instantiated.
 **/

#pragma once

#include "pretty.hpp"
#include "FlatSink.hpp"
#include <ostream>

namespace xo::pp {

    /** Write @p x on stream @p os, using a FlatSink.
     *  Equivalent to os << x, except that it can use
     *  Prettifier<T>::print() where available,
     *  to determine printing.
     *
     *  Although it uses pretty-printing infra,
     *  this path will not pretty-print.
     *
     *  May use with pretty-enabled classes to provide
     *  legacy operator<<. E.g. see webutil_ostream.hpp
     *
     *  For bona-fide pretty-printing see xo-indentlog2::PrettySink
     **/
    template <typename T>
    inline std::ostream &
    pp_to_stream(std::ostream & os, const T & x) {
        FlatSink sink(os);
        sink.pp(x);
        return os;
    }

}

/* end pretty_ostream.hpp */
