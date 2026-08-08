/* file pretty_parserstatemachine.hpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#pragma once

#include "parserstatemachine.hpp"
#include <xo/ppsink/pretty.hpp>

namespace xo::pp {
    /* parserstatemachine is not in the expression hierarchy: no virtual,
     * these render it directly.  Defined in pretty_parserstatemachine.cpp.
     */
    template <>
    struct Prettifier<xo::scm::parserstatemachine> {
        static void print(PpSink & sink, const xo::scm::parserstatemachine & x);
    };

    template <>
    struct Prettifier<xo::scm::parserstatemachine *> {
        static void print(PpSink & sink, const xo::scm::parserstatemachine * x);
    };
} /*namespace xo::pp*/

/* end pretty_parserstatemachine.hpp */
