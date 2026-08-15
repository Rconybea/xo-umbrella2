/** @file tostr.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  xo::pp::tostr(args...): build a std::string by rendering @p args, in order,
 *  through a FlatSink wrapping a std::stringstream.
 *
 *  The ppsink replacement for legacy xo::tostr (xo/indentlog/print/tostr.hpp),
 *  which is ubiquitous across xo for one-off strings -- error messages, labels.
 *  Intended for a SMALL amount of flat text: FlatSink flattens any structure,
 *  so pretty-printing is deliberately not offered here.  Each arg renders via
 *  pretty() -- Prettifier<T> if it has one, else its operator<< (hence the
 *  <ostream>-bearing pretty_ostream.hpp).
 *
 *  NB: this uses std::stringstream / std::string (heap).  An *arena-backed*
 *  tostr (building the result into arena memory) is a separate, higher-level
 *  facility -- it cannot live here because xo-ppsink sits below xo-arena in the
 *  level order.
 **/

#pragma once

#include "FlatSink.hpp"
#include "pretty_ostream.hpp" /* pretty() + <ostream> for the operator<< fallback */
#include <sstream>
#include <string>

namespace xo::pp {
    /** render @p args (concatenated, no separator) to a std::string **/
    template <typename... Ts>
    std::string
    tostr(const Ts &... args) {
        std::stringstream ss;
        FlatSink sink(ss.rdbuf());
        (sink.pp(args), ...);
        return ss.str();
    }
} /*namespace xo::pp*/

/* end tostr.hpp */
