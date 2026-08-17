/* @file pretty_expression.hpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#pragma once

#include "Expression.hpp"
#include <xo/ppsink/pretty.hpp>

namespace xo::pp {
    /* the hierarchy dispatches dynamically through the virtual
     * GeneralizedExpression::pretty(); these just enter it.
     */
    template <>
    struct Prettifier<xo::scm::GeneralizedExpression> {
        static void print(PpSink & sink, const xo::scm::GeneralizedExpression & x) {
            x.pretty(sink);
        }
    };

    template <>
    struct Prettifier<xo::scm::Expression> {
        static void print(PpSink & sink, const xo::scm::Expression & x) {
            x.pretty(sink);
        }
    };
} /*namespace xo::pp*/

/* end pretty_expression.hpp */
