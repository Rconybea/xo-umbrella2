/** @file DInteger.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <xo/alloc2/GCObjectVisitor.hpp>
#include <xo/alloc2/Allocator.hpp>
#include <xo/indentlog/print/ppindentinfo.hpp>
#include <xo/facet/obj.hpp>
#include <cstdint>

namespace xo {
    namespace scm {
        struct DInteger {
            using AAllocator = xo::mm::AAllocator;
            using AGCObjectVisitor = xo::mm::AGCObjectVisitor;
            using AGCObject = xo::mm::AGCObject;
            using VisitReason = xo::mm::VisitReason;
            using ppindentinfo = xo::print::ppindentinfo;
            using value_type = long;

            explicit DInteger(long x) : value_{x} {}

            /** will likely want this to default to ANumeric, once we have it **/
            template <typename AFacet = AGCObject>
            static obj<AFacet, DInteger> box(obj<AAllocator> mm, long x);

            /** allocate boxed value @p x using memory from @p mm **/
            static DInteger * _box(obj<AAllocator> mm, long x);

            long value() const noexcept { return value_; }

            bool pretty(const ppindentinfo & ppii) const;

            operator long() const noexcept { return value_; }

            void assign_value(long x) noexcept { this->value_ = x; }

            // GCObject facet

            DInteger * gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept;
            void visit_gco_children(VisitReason reason, obj<AGCObjectVisitor> gc) noexcept;

        private:
            /** boxed integer value **/
            long value_;
        };

        template <typename AFacet>
        obj<AFacet, DInteger>
        DInteger::box(obj<AAllocator> mm, long x) {
            return obj<AFacet,DInteger>(_box(mm, x));
        }

    } /*nmaespace obj*/
} /*namespace xo*/

/* end DInteger.hpp */
