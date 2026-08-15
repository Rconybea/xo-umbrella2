/** @file alloc_ostream.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  operator<<(std::ostream&, gp<Object>) -- the paved road to std::ostream.
 *
 *  xo::Object renders into a xo::pp::PpSink (see Object::pretty() and
 *  Prettifier<gp<Object>> in Object.hpp), and Object.hpp names no std::ostream
 *  at all.  Within xo, PpSink is the intended path: an object nests inside an
 *  enclosing pretty_struct and participates in its line breaking, where an
 *  ostream inserter flattens it into one unbreakable token.
 *
 *  But a PpSink is not what someone reaching for xo the first time already has
 *  in their hand -- they have a std::ostream.  So rather than making them
 *  discover FlatSink, this header gives them the inserter, implemented over
 *  FlatSink exactly as they would have written it:
 *
 *  @code
 *    #include <xo/alloc/alloc_ostream.hpp>
 *    std::cout << obj << std::endl;
 *  @endcode
 *
 *  Equivalent, without this header:
 *  @code
 *    xo::pp::FlatSink sink(std::cout.rdbuf());
 *    sink.pp(obj);
 *  @endcode
 *
 *  Expected usage inside xo: unit tests only.  New xo code that wants to print
 *  an object should render into the PpSink it already has.
 *
 *  FlatSink renders splits as their flat spaces and never breaks a line, so
 *  output here is the single-line form regardless of length -- which is what
 *  the pre-conversion display(std::ostream&) produced, byte for byte.
 *
 *  Follows the pattern of xo-webutil/include/xo/webutil/webutil_ostream.hpp.
 **/

#pragma once

#include "Object.hpp"
#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/pretty.hpp>   /* PpSink::pp -- declared in PpSink.hpp,
                                   * defined here (it needs Prettifier).
                                   * Same include the other xo *_ostream.hpp
                                   * headers carry for this reason.
                                   */
#include <ostream>

namespace xo {
    /** print @p x on @p os, in its flat (never-broken) form.
     *
     *  Goes through Prettifier<gp<Object>>, so the null case is decided in one
     *  place (Object.hpp) rather than again here.
     **/
    inline std::ostream &
    operator<< (std::ostream & os, gp<Object> x) {
        xo::pp::FlatSink sink(os.rdbuf());

        sink.pp(x);

        return os;
    }
} /*namespace xo*/

/* end alloc_ostream.hpp */
