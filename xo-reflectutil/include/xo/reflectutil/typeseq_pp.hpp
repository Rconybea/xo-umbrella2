/** @file typeseq_pp.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  ppsink-native structured printing for xo::reflect::typeseq.
 *
 *  Split out of typeseq.hpp per the tree's <thing>_pp.hpp convention (cf.
 *  xo/arena/span_pp.hpp, xo/reflect/TypeDescr_pp.hpp), so typeseq.hpp itself
 *  carries no pretty-printing vocabulary and code that wants only the type
 *  identity does not pay for the printing machinery.
 *
 *  Why this exists at all: without a Prettifier<>, a typeseq reached ppsink's
 *  operator<< FALLBACK (pretty.hpp's third dispatch branch), which compiles and
 *  renders correctly but goes through an ostream -- and does so silently, since
 *  a type with an operator<< gives no diagnostic when it lands there.
 *  xo-reflectutil is used by ~50 subsystems, so that was very likely the widest
 *  ostream dependency in the tree.
 *
 *  There is deliberately no typeseq_ostream.hpp.  Every streaming use of a
 *  typeseq in the tree (measured 2026-08-10, ~140 sites) is a TAG VALUE --
 *  xtag("x.tseq", x._typeseq()) -- which reaches this Prettifier through the
 *  tag machinery.  Nothing does a bare `os << tseq`.  If a call site ever needs
 *  one, xo::pp::pp_to_stream(os, x) (pretty_ostream.hpp) is the generic answer;
 *  a dedicated inserter, per quoted_ostream.hpp, is the fallback.
 *
 *  Output is unchanged: the bare seqno, exactly as the legacy inserter emitted,
 *  but via Prettifier<int32_t> (std::to_chars) rather than an ostream.
 **/

#pragma once

#include "typeseq.hpp"
#include <xo/ppsink/pretty.hpp> /* Prettifier<>, PpSink::pp */

namespace xo::pp {
    /** @brief structured pretty-printing for xo::reflect::typeseq.
     *
     *  A typeseq is a single int32_t, so this is a leaf: it delegates to the
     *  integer Prettifier rather than framing anything.  Nothing here breaks
     *  lines, which is correct -- a type id is not a structure.
     **/
    template <>
    struct Prettifier<xo::reflect::typeseq> {
        static void print(PpSink & sink, const xo::reflect::typeseq & x) {
            sink.pp(x.seqno());
        }
    };
} /*namespace xo::pp*/

/* end typeseq_pp.hpp */
