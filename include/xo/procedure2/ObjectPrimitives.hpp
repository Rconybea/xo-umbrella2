/** @file ObjectPrimitives.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

#include "Primitive_gco_0.hpp"
#include "Primitive_gco_2_gco_gco.hpp"
#include "Primitive_gco_3_dict_string_gco.hpp"

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
            /** create primitive: report current working directory **/
            static DPrimitive_gco_0 * make_cwd_pm(obj<AAllocator> mm);

            /** create primitive: fetch nth element of a sequence **/
            static DPrimitive_gco_2_gco_gco * make_nth_pm(obj<AAllocator> mm);

            /** create primitive: create cons cell **/
            static DPrimitive_gco_2_gco_gco * make_cons_pm(obj<AAllocator> mm);

            /** create primitive for creating a dictionary instance **/
            static DPrimitive_gco_0 * make_dict_make_pm(obj<AAllocator> mm);

            /** create primitive that upserts a key,value pair into a dictionary **/
            static DPrimitive_gco_3_dict_string_gco * make_dict_upsert_pm(obj<AAllocator> mm);
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end ObjectPrimitives.hpp */
