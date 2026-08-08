/* file pretty_envframestack.hpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#pragma once

#include "envframestack.hpp"
#include <xo/ppsink/pretty.hpp>

namespace xo::pp {
    template <>
    struct Prettifier<const xo::scm::envframestack *> {
        static void print(PpSink & sink, const xo::scm::envframestack * p) {
            if (p) p->pretty(sink); else sink.put("<nullptr envframestack>");
        }
    };

    template <>
    struct Prettifier<xo::scm::envframestack *> {
        static void print(PpSink & sink, const xo::scm::envframestack * p) {
            if (p) p->pretty(sink); else sink.put("<nullptr envframestack>");
        }
    };
} /*namespace xo::pp*/

/* end pretty_envframestack.hpp */
