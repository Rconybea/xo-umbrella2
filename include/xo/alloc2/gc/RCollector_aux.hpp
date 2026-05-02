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

    } /*namespace mm*/
} /*namespace xo*/

/* end RCollector_aux.hpp */
