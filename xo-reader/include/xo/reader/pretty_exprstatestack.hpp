/* file pretty_exprstatestack.hpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#pragma once

#include "exprstatestack.hpp"
#include <xo/ppsink/pretty.hpp>

namespace xo::pp {
    template <>
    struct Prettifier<xo::scm::exprstatestack *> {
        static void print(PpSink & sink, const xo::scm::exprstatestack * p) {
            if (p) p->pretty(sink); else sink.put("<nullptr exprstatestack>");
        }
    };

    template <>
    struct Prettifier<const xo::scm::exprstatestack *> {
        static void print(PpSink & sink, const xo::scm::exprstatestack * p) {
            if (p) p->pretty(sink); else sink.put("<nullptr exprstatestack>");
        }
    };

    template <>
    struct Prettifier<xo::scm::exprstate *> {
        static void print(PpSink & sink, const xo::scm::exprstate * p) {
            if (p) p->pretty(sink); else sink.put("<nullptr exprstate>");
        }
    };
} /*namespace xo::pp*/

/* end pretty_exprstatestack.hpp */
