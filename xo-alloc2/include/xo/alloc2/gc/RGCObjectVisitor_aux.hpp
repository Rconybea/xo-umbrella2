/** @file RGCObjectVisitor_aux.hpp
 *
 *  Out-of-line definitions for RCollector template methods
 *  that depend on RGCObject (avoiding #include cycle in RCollector.hpp).
 *
 *  Included via user_hpp_includes in GCObject.json5.
 *
 *  @author Roland Conybeare
 **/

#pragma once

#include "gc/RGCObjectVisitor.hpp"
#include <xo/facet/FacetRegistry.hpp>

namespace xo {
    namespace mm {
        class ACollector;
        class AGCObject;

        /** defined here to avoid #include cycle, since
         *  template obj<AGCObject,DRepr> awkward to make available
         *  in RCollector.hpp
         **/
        template <typename Object>
        template <typename DRepr>
        void
        RGCObjectVisitor<Object>::visit_child(VisitReason reason,
                                              xo::facet::obj<AGCObject,DRepr> * p_obj)
        {
            this->visit_child(reason, p_obj->iface(), (void **)&(p_obj->data_));
        }

        template <typename Object>
        template <typename DRepr>
        void
        RGCObjectVisitor<Object>::visit_child(VisitReason reason, DRepr ** p_repr)
        {
            // fetch static interface for DRepr (strip const: FacetImplementation specializations use non-const DRepr)
            auto iface = xo::facet::impl_for<AGCObject, std::remove_const_t<DRepr>>();

            this->visit_child(reason, &iface, (void **)p_repr);
        }

        template <typename Object>
        template <typename AFacet, typename DRepr>
        requires (!std::is_same_v<AFacet, AGCObject>)
        void
        RGCObjectVisitor<Object>::visit_poly_child(VisitReason reason, obj<AFacet, DRepr> * p_objs)
        {
            if (*p_objs) {
                auto e = xo::facet::FacetRegistry::instance().variant<AGCObject,AFacet>(*p_objs);

                this->visit_child(reason, e.iface(), (void **)&(p_objs->data_));
            }
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end RCollector_aux.hpp */
