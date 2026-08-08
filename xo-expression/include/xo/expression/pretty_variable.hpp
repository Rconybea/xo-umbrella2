/* file pretty_variable.hpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#pragma once

#include "Variable.hpp"
#include "pretty_expression.hpp"

namespace xo::pp {
    template <>
    struct Prettifier<xo::scm::Variable> {
        static void print(PpSink & sink, const xo::scm::Variable & x) {
            x.pretty(sink);
        }
    };

    template <>
    struct Prettifier<xo::scm::Variable *> {
        static void print(PpSink & sink, const xo::scm::Variable * x) {
            if (x)
                x->pretty(sink);
            else
                sink.put("<nullptr Variable>");
        }
    };
} /*namespace xo::pp*/

/* end pretty_variable.hpp */
