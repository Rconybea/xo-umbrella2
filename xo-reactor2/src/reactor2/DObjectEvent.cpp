/** @file DObjectEvent.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#include "event/DObjectEvent.hpp"
#include <xo/printable2/Printable.hpp>
#include <xo/ppsink/pretty_struct.hpp>

namespace xo {
    using xo::print::APrintable;
    using xo::mm::AGCObjectVisitor;
    using xo::pp::field;

    namespace process {

    void
    DObjectEvent::reflect_self()
    {
        using xo::reflect::StructReflector;

        StructReflector<DObjectEvent> sr;

        if (sr.is_incomplete()) {
            REFLECT_MEMBER(sr, tm);
            REFLECT_MEMBER(sr, value);
        }
    }

    // ----- Printable facet -----

    void
    DObjectEvent::pretty(PpSink & sink) const
    {
        // auto value = const_cast<DObjectEvent*>(this)->value_....
        auto value = value_.to_facet<APrintable>();

        sink.pretty_struct("DObjectEvent",
                           field("tm", tm_),
                           field("value", value, bool(value)));
    }

    // ----- GCObject facet -----

    DObjectEvent *
    DObjectEvent::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
    {
        return gc.std_move_for(this);
    }

    void
    DObjectEvent::visit_gco_children(VisitReason reason,
                                     obj<AGCObjectVisitor> gc) noexcept
    {
        gc.visit_child(reason, &value_);
    }

    } /*namespace process*/
} /*namespace xo*/

/* end DObjectEvent.cpp */
