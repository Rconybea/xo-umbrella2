/** @file ObjectPrimitives.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

#include <xo/procedure2/DPrimitive_gco_2_gco_gco.hpp>

namespace xo {
    namespace scm {
        /** @brief primitives centered on object2/ data structures.
         *
         *  Note: they don't reside in object2/ because DPrimitive
         *  not available yet at that level
         **/
        class ObjectPrimitives {
        public:
            using AAllocator = xo::mm::AAllocator;

        public:
            /** create primitive for fetching nth element of a sequence **/
            static DPrimitive_gco_2_gco_gco * make_nth_pm(obj<AAllocator> mm);
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end ObjectPrimitives.hpp */
