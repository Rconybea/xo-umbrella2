/** @file tostr0.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  xo::pp::tostr(args...): build a std::string by rendering @p args, in order,
 *  through a FlatSink wrapping a std::stringstream.
 *
 *  Implementation here is tactical: it is superseded by xo-indentlog2
 *  functions xo::pp::tostr / xo::pp::pptostr
 *
 *  Reserve tostr0() for subsystems that cannot use the xo-indentlog2
 *  because of leveling topology:
 *    xo-testutil xo-arena xo-subsys xo-randomgen
 *  Also a couple of unforced choices for principle of least surprise:
 *    xo-refcnt xo-flatstring
 **/

#pragma once

#include "FlatSink.hpp"
//#include "pretty_ostream.hpp" /* pretty() + <ostream> for the operator<< fallback */
#include "pretty.hpp"
#include <sstream>
#include <string>

namespace xo::pp {
    /** render @p args (concatenated, no separator) to a std::string **/
    template <typename... Ts>
    std::string
    tostr0(const Ts &... args) {
        std::stringstream ss;
        FlatSink sink(ss.rdbuf());
        (sink.pp(args), ...);
        return ss.str();
    }
} /*namespace xo::pp*/

/* end tostr0.hpp */
