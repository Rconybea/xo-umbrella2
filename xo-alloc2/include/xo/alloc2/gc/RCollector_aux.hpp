/** @file RCollector_aux.hpp
 *
 *  Out-of-line definitions for RCollector template methods
 *  that depend on RGCObject (avoiding #include cycle in RCollector.hpp).
 *
 *  Included via user_hpp_includes in GCObject.json5.
 *
 *  @author Roland Conybeare
 **/

#pragma once

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
        RGCObjectVisitor<Object>::visit_child(xo::facet::obj<AGCObject,DRepr> * p_obj)
        {
            this->visit_child(p_obj->iface(), (void **)&(p_obj->data_));
        }

        template <typename Object>
        template <typename DRepr>
        void
        RGCObjectVisitor<Object>::visit_child(DRepr ** p_repr)
        {
            // fetch static interface for DRepr (strip const: FacetImplementation specializations use non-const DRepr)
            auto iface = xo::facet::impl_for<AGCObject, std::remove_const_t<DRepr>>();

            this->visit_child(&iface, (void **)p_repr);
        }

        template <typename Object>
        template <typename AFacet, typename DRepr>
        requires (!std::is_same_v<AFacet, AGCObject>)
        void
        RGCObjectVisitor<Object>::visit_poly_child(obj<AFacet, DRepr> * p_objs)
        {
            if (*p_objs) {
                auto e = xo::facet::FacetRegistry::instance().variant<AGCObject,AFacet>(*p_objs);

                this->visit_child(e.iface(), (void **)&(p_objs->data_));
            }
        }

        // ----- DEPRECATED -----
        //
        // Moving these methods to RGCObjectVisitor

        /** defined here to avoid #include cycle, since
         *  template obj<AGCObject,DRepr> awkward to make available
         *  in RCollector.hpp
         **/
        template <typename Object>
        template <typename DRepr>
        void
        RCollector<Object>::forward_inplace(xo::facet::obj<AGCObject,DRepr> * p_obj)
        {
            this->forward_inplace(p_obj->iface(), (void **)&(p_obj->data_));
        }

        template <typename Object>
        template <typename DRepr>
        void
        RCollector<Object>::forward_inplace(DRepr ** p_repr)
        {
            // fetch static interface for DRepr (strip const: FacetImplementation specializations use non-const DRepr)
            auto iface = xo::facet::impl_for<AGCObject, std::remove_const_t<DRepr>>();

            this->forward_inplace(&iface, (void **)p_repr);
        }

        template <typename Object>
        template <typename AFacet, typename DRepr>
        requires (!std::is_same_v<AFacet, AGCObject>)
        void
        RCollector<Object>::forward_pivot_inplace(obj<AFacet, DRepr> * p_objs)
        {
            if (*p_objs) {
                auto e = xo::facet::FacetRegistry::instance().variant<AGCObject,AFacet>(*p_objs);
                this->forward_inplace(e.iface(), (void **)&(p_objs->data_));
            }
        }

        // ----- mm_do_assign -----

        /** gc-aware assignment; engage special book-keeping for cross-gen pointers **/
        inline void mm_do_assign(obj<ACollector> & gc,
                                 void * parent,
                                 obj<AGCObject> * p_lhs,
                                 obj<AGCObject> & rhs)
        {
            if (gc.data()) {
                gc.assign_member(parent, p_lhs, rhs);
            } else {
                // assume null collector downstream from allocator that does not provide collection.
                // In that no additional assignment work.

                *p_lhs = rhs;
            }
        };
    }
}

/* end RCollector_aux.hpp */
