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

namespace xo {
    namespace mm {
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
            // fetch static interface for DRepr
            auto iface = xo::facet::impl_for<AGCObject,DRepr>();

            this->forward_inplace(&iface, (void **)p_repr);
        }
    }
}

/* end RCollector_aux.hpp */
