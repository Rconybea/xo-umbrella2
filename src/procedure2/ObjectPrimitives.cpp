/** @file ObjectPrimitives.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "ObjectPrimitives.hpp"
#include "Primitive_gco_2_gco_gco.hpp"
#include <xo/object2/Sequence.hpp>
#include <xo/object2/Integer.hpp>

namespace xo {
    using xo::scm::ASequence;
    using xo::mm::AAllocator;
    using xo::mm::AGCObject;

    namespace scm {

        // TODO: seq_gc -> obj<ASequence>
        //       n_gco -> obj<AGCObject,DInteger>
        //
        obj<AGCObject>
        xfer_nth(obj<ARuntimeContext> rcx,
                 obj<AGCObject> seq_gco,
                 obj<AGCObject> n_gco)
        {
            scope log(XO_DEBUG(true));

            (void)rcx;

            obj<ASequence> seq = seq_gco.to_facet<ASequence>();
            auto n = obj<AGCObject,DInteger>::from(n_gco);

            return seq.at(n->value());
        }

        DPrimitive_gco_2_gco_gco *
        ObjectPrimitives::make_nth_pm(obj<AAllocator> mm)
        {
            return DPrimitive_gco_2_gco_gco::_make(mm, "nth", &xfer_nth);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end ObjectPrimitives.cpp */
