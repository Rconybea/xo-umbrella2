/** @file DObjectEvent.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#pragma once

#include "DTypedEvent.hpp"
#include <xo/alloc2/GCObject.hpp>
#include <xo/timeutil/timeutil.hpp>

namespace xo::process {
    class DObjectEvent : public DTypedEvent<obj<xo::mm::AGCObject>>
    {
    public:
        using ParentType = DTypedEvent<obj<xo::mm::AGCObject>>;
        using AGCObject = xo::mm::AGCObject;
        using AGCObjectVisitor = xo::mm::AGCObjectVisitor;
        using VisitReason = xo::mm::VisitReason;

    public:
        DObjectEvent() = default;
        DObjectEvent(utc_nanos tm, const obj<AGCObject> & x) : ParentType{tm, {x}} {}

        /** reflect DObjectEvent object representation **/
        static void reflect_self();

        // APrintable facet

        void pretty(PpSink & sink) const;

        // AGCObject facet

        DObjectEvent * gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept;
        void visit_gco_children(VisitReason, obj<AGCObjectVisitor> gc) noexcept;
    };
}

/* end DObjectEvent.hpp */
