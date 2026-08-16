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
        FlatSink sink(os.rdbuf());
        sink.pp(x);
        return os;
    }

    /** convenience so given a PpSinkInserter:
     *    PpSinkInserter ins = ppsink.stream_open(..);
     *  we can write
     *    ins << x << ...;
     *  instead of
     *    ins.os() << x << ...;
     **/
    /** NB deliberately UNconstrained.  `requires requires (std::ostream & os,
     *  const T & x) { os << x; }` looks right and does not compile: for a T in
     *  namespace xo::pp (tag_impl, quot_impl, ..) ADL brings every xo::pp
     *  operator<< into that overload resolution -- including this one -- so the
     *  constraint's satisfaction depends on itself, which gcc rejects outright.
     *  std::ostream never converts to PpSinkInserter&, but constraint checking
     *  happens before that is determined.
     *
     *  Consequence: xo::pp::pp_streamable<T> (pretty.hpp) answers "did this TU
     *  include pretty_ostream.hpp", not "is T streamable".  A T with no
     *  operator<<(std::ostream&, const T&) therefore fails inside this body
     *  rather than at pretty()'s static_assert.
     **/
    template <typename T>
    inline PpSinkInserter &
    operator<<(PpSinkInserter & ins, const T & x) {
        std::ostream os(ins.sbuf());
        os << x;
        return ins;
    }
}

/* end pretty_ostream.hpp */
