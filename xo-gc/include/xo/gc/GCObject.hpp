/** @file GCObject.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/Users/roland/proj/xo-umbrella2/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/GCObject.json5]
 *  2. jinja2 template for facet .hpp file:
 *       [facet.hpp.j2]
 *  3. idl for facet methods
 *       [idl/GCObject.json5]
 **/

#pragma once

#include "detail/AGCObject.hpp"
#include "detail/IGCObject_Any.hpp"
#include "detail/IGCObject_Xfer.hpp"
#include "detail/RGCObject.hpp"

namespace xo {
    namespace mm {
        /** defined here to avoid #include cycle, since
         *  template obj<AGCObject,DRepr> awkward to make available there
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

/* end GCObject.hpp */

