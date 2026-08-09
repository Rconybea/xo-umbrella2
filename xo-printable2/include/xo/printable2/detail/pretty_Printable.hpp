/** @file pretty_Printable.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  Prettifier<obj<APrintable, DRepr>> -- render a printable object into a
 *  PpSink via its pretty() method.
 *
 *  The ppsink counterpart to ppdetail_Printable.hpp, which does the same for
 *  the deprecated two-pass protocol:
 *
 *    ppdetail<obj<APrintable, DRepr>>::print_pretty  -> x.pretty_deprecated(ppii)
 *    Prettifier<obj<APrintable, DRepr>>::print       -> x.pretty(sink)
 *
 *  Without this, handing a printable object to a sink (sink.pp(x), xtag("k", x),
 *  field("k", x)) falls through Prettifier's empty primary template to
 *  operator<<, which is not what any of those callers mean.
 *
 *  See .xo-backlog/xo-printable2/issues/01-aprintable-pretty-ppsink.md
 **/

#pragma once

#include "xo/printable2/Printable.hpp"
#include <xo/ppsink/Prettifier.hpp>

namespace xo::pp {
    template <typename DRepr>
    struct Prettifier<xo::facet::obj<xo::print::APrintable, DRepr>> {
        static void print(PpSink & sink,
                          const xo::facet::obj<xo::print::APrintable, DRepr> & x) {
            x.pretty(sink);
        }
    };
} /*namespace xo::pp*/

/* end pretty_Printable.hpp */
