/** @file TypeDescr_pp.hpp
 *
 *  ppsink-native structured printing for xo::reflect::TypeDescrBase and
 *  TypeDescr (= const TypeDescrBase *).
 *
 *  Split out of TypeDescr.hpp per the tree's <thing>_pp.hpp convention (cf.
 *  xo/arena/span_pp.hpp), so TypeDescr.hpp itself carries no pretty-printing
 *  vocabulary and consumers that only need the type description do not pay
 *  for the printing machinery.
 *
 *  See TypeDescr_ppdetail.hpp for the legacy xo-indentlog equivalent.
 *
 *  Both specializations forward to TypeDescrBase::pretty(), defined out of
 *  line in TypeDescr.cpp -- so this header stays ostream-free even though the
 *  fields it ultimately prints (TypeId, Metatype, bool) render through the
 *  operator<< fallback.
 **/

#pragma once

#include "TypeDescr.hpp"
#include <xo/ppsink/Prettifier.hpp>

namespace xo::pp {
    template <>
    struct Prettifier<xo::reflect::TypeDescrBase> {
        static void print(PpSink & sink, const xo::reflect::TypeDescrBase & td) {
            td.pretty(sink);
        }
    };

    /** TypeDescr is a pointer (const TypeDescrBase *).
     *
     *  A null descriptor prints nothing, preserving the legacy
     *  ppdetail<TypeDescr> behaviour (`return td ? td->pretty(ppii) : true`).
     *  Printing "<null>" instead would arguably be friendlier, but that is an
     *  output-visible change and belongs in its own commit.
     **/
    template <>
    struct Prettifier<xo::reflect::TypeDescr> {
        static void print(PpSink & sink, xo::reflect::TypeDescr td) {
            if (td)
                td->pretty(sink);
        }
    };
} /*namespace xo::pp*/

/* end TypeDescr_pp.hpp */
