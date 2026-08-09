/** @file DBoolean.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <xo/alloc2/Allocator.hpp>
#include <xo/alloc2/GCObjectVisitor.hpp>
#include <xo/facet/obj.hpp>
#include <xo/indentlog/print/ppindentinfo.hpp>
#include <cstdint>

namespace xo {
    namespace scm {
        struct DBoolean {
            using AAllocator = xo::mm::AAllocator;
            using AGCObjectVisitor = xo::mm::AGCObjectVisitor;
            using AGCObject = xo::mm::AGCObject;
            using VisitReason = xo::mm::VisitReason;
            using ppindentinfo = xo::print::ppindentinfo;
            using value_type = long;

            explicit DBoolean(bool x) : value_{x} {}

            /** will likely want this to default to ANumeric, once we have it **/
            template <typename AFacet = AGCObject>
            static obj<AFacet, DBoolean> box(obj<AAllocator> mm, bool x);

            /** allocate boxed value @p x using memory from @p mm **/
            static DBoolean * _box(obj<AAllocator> mm, bool x);

            bool value() const noexcept { return value_; }

            bool pretty_deprecated(const ppindentinfo & ppii) const;

            /* PHASE B STUB -- renders nothing until phase C converts this type.
             * See .xo-backlog/xo-printable2/issues/01-aprintable-pretty-ppsink.md
             */
            void pretty(xo::pp::PpSink & sink) const { (void)sink; }

            operator bool() const noexcept { return value_; }

            // GCObject facet

            DBoolean * gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept;
            void visit_gco_children(VisitReason reason, obj<AGCObjectVisitor> gc) noexcept;

        private:
            /** boxed boolean value **/
            bool value_ = false;
        };

        template <typename AFacet>
        obj<AFacet, DBoolean>
        DBoolean::box(obj<AAllocator> mm, bool x) {
            return obj<AFacet,DBoolean>(_box(mm, x));
        }

    } /*nmaespace obj*/
} /*namespace xo*/

/* end DBoolean.hpp */
