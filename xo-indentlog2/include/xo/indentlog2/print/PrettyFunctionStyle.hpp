/** @file PrettyFunctionStyle.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include <xo/ppsink/FunctionStyle.hpp>
#include <xo/ppsink/Prettifier.hpp>

namespace xo::pp {
    /** pretty-print a FunctionStyle as its descriptive name.
     *  Atomic: no group structure.
     **/
    template <>
    struct Prettifier<xo::FunctionStyle> {
        static void print(PpSink & sink, xo::FunctionStyle x) {
            sink.put(descr_of(x));   /* ADL: xo::descr_of */
        }
    };
} /*namespace xo::pp*/

/* end PrettyFunctionStyle.hpp */
