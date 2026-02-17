/** @file PolyForwarderUtil.hpp
 *
 *  @author Roland Conybeare, Feb 2026
 **/

#pragma once

#include "Collector.hpp"
#include <xo/facet/FacetRegistry.hpp>

namespace xo {
    namespace mm {

        /** Utility class for forwarding support on
         *  faceted object pointers that have some primary
         *  facet _other_ than AGCObject.
         *
         *  For fop with AGCObject facet, with collector gc:
         *
         *    obj<ACollector> gc = ..;
         *    obj<AGCObject> ptr = ..;
         *
         *    gc.forward_inplace(&ptr);
         *
         *  for fop with some other facet:
         *
         *    obj<ASomething> ptr = ..;
         *    PolyForwarderUtil::forward_inplace(gc, &ptr);
         *
         *  or
         *    poly_forward_inplace(gc, &ptr);
         **/
        class PolyForwarderUtil {
        public:
            template <typename AFacet, typename DRepr>
            static void forward_inplace(obj<ACollector> gc, obj<AFacet,DRepr> * p_ptr) {
                using xo::facet::FacetRegistry;

                /**
                 *    p_ptr
                 *    v                       FacetRegistry
                 *    +--------+---------+      .variant()        +-----------+---------+
                 *    | AFacet | DRepr x |    ----------------->  | AGCobject | DRepr x |
                 *    +--------+-------|-+                        +-----------+-------|-+
                 *                     |                                              |
                 *                     |  /-------------------------------------------/
                 *                     |  |
                 *                     v  v
                 *                   +-------+
                 *                   | DRepr |
                 *                   +-------+
                 **/

                auto gco = FacetRegistry::instance().variant<AGCObject,AFacet>(*p_ptr);
                gc.forward_inplace(gco.iface(), (void **)&(p_ptr->data_));
            }
        };

        template <typename AFacet, typename DRepr>
        void poly_forward_inplace(obj<ACollector> gc, obj<AFacet, DRepr> * p_ptr) {
            PolyForwarderUtil::forward_inplace(gc, p_ptr);
        }
    } /*namespace mm*/
} /*namespace xo*/

/* end PolyForwarderUtil.hpp */
