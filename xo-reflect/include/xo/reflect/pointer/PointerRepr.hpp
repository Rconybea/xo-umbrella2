/** @file PointerRepr.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

namespace xo {
    namespace reflect {
        /** @brief which kind of pointer-like type a Metatype::mt_pointer is.
         *
         *  One metatype for "refers to one object"; this tells the kinds
         *  apart, for consumers that care.  Ownership (child_edge_ownership)
         *  is separate: it tells rp<T> (shared) from std::unique_ptr<T>
         *  (owning).  See .xo-backlog/std-function-reflection/spec.md, 1b
         **/
        enum class PointerRepr {
            /** T * **/
            raw,
            /** a smart pointer: xo::ref::rp<T>, std::unique_ptr<T, D> **/
            smart,
            /** T & -- described, not traversed **/
            lvalue_reference,
            /** T && -- described, not traversed **/
            rvalue_reference,
            /** a faceted object pointer, xo::facet::obj<AFacet, DRepr>: a facet
             *  pointer plus a data pointer.  Says nothing of how the data is
             *  managed -- that is the representation's business
             **/
            fop,
        };
    } /*namespace reflect*/
} /*namespace xo*/

/** end PointerRepr.hpp **/
