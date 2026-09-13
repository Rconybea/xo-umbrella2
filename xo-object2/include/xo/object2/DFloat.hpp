/** @file DFloat.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <xo/alloc2/Allocator.hpp>
#include <xo/alloc2/GCObjectVisitor.hpp>
#include <xo/facet/obj.hpp>
#include <xo/reflect/TaggedPtr.hpp>

namespace xo {
    namespace scm {
        struct DFloat {
            using AAllocator = xo::mm::AAllocator;
            using AGCObjectVisitor = xo::mm::AGCObjectVisitor;
            using VisitReason = xo::mm::VisitReason;
            using value_type = double;

            explicit DFloat(double x) : value_{x} {}

            /** probably want default = ANumeric, once we introduce it **/
            template <typename AFacet>
            static obj<AFacet,DFloat> box(obj<AAllocator> mm, double x);

            /** allocate boxed value @p x using memory from @p mm **/
            static DFloat * _box(obj<AAllocator> mm, double x);

            double value() const noexcept { return value_; }

            operator double() const noexcept { return value_; }


            /** structured pretty-printing: render this float into @p sink **/
            void pretty(xo::pp::PpSink & sink) const;

            /** reflection: hand back a TaggedPtr for this DFloat.
             *
             *  Faithful to the REPRESENTATION -- reflection describes what is
             *  there, and a consumer that wants a boxed float to read as a
             *  bare number says so with its own printer, rather than having
             *  reflection lie about the layout.
             *
             *  Requires @ref reflect_self to have run.
             *
             *  Borrowed, like every fop TaggedPtr: valid while the arena
             *  holding this DFloat is.
             **/
            xo::reflect::TaggedPtr self_tp();

            /** describe DFloat's layout to xo-reflect.  Member-wise, so it
             *  must live here: value_ is private.
             *
             *  Note this is SEPARATE from how a DFloat renders as JSON, which
             *  is xo-printjson's business and is a bare number -- see
             *  SetupObject2::provide_json_printers.  Reflection describes what
             *  is there; a consumer decides what to make of it.
             **/
            static void reflect_self();

            // GCObject facet
            DFloat * gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept;
            void visit_gco_children(VisitReason reason, obj<AGCObjectVisitor> gc) noexcept;

        private:

            /** boxed floating-oint value **/
            double value_;
        };

        template <typename AFacet>
        obj<AFacet,DFloat>
        DFloat::box(obj<AAllocator> mm, double x) {
            return obj<AFacet,DFloat>(DFloat::_box(mm, x));
        }
    } /*nmaespace scm*/
} /*namespace xo*/

/* end DFloat.hpp */
