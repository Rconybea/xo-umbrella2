/* file pretty_parserstatemachine.cpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#include "pretty_parserstatemachine.hpp"
#include "exprstatestack.hpp"
#include "pretty_envframestack.hpp"
#include "pretty_exprstatestack.hpp"
#include <xo/ppsink/pretty_struct.hpp>

namespace xo::pp {
    void
    Prettifier<xo::scm::parserstatemachine>::print(PpSink & sink,
                                                   const xo::scm::parserstatemachine & x)
    {
        /* parserstatemachine is not part of the expression hierarchy, so there
         * is no virtual involved -- this is a plain Prettifier.
         */
        sink.pretty_struct("psm",
                           field("stack", x.xs_stack_),
                           field("env_stack", x.env_stack_));
    }

    void
    Prettifier<xo::scm::parserstatemachine *>::print(PpSink & sink,
                                                     const xo::scm::parserstatemachine * x)
    {
        if (x)
            Prettifier<xo::scm::parserstatemachine>::print(sink, *x);
        else
            sink.put("<nullptr>");
    }
} /*namespace xo::pp*/

/* end pretty_parserstatemachine.cpp */
