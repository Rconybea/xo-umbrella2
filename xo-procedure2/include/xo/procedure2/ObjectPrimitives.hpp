/** @file ObjectPrimitives.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

#include "Primitive_gco_0.hpp"
#include "Primitive_gco_1_gco.hpp"
#include "Primitive_gco_2_gco_gco.hpp"
#include "Primitive_gco_2_dict_string.hpp"
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
            static DPrimitive_gco_0 * make_cwd_pm(obj<AAllocator> mm,
                                                  StringTable * stbl);

            /** create primitive: fetch nth element of a sequence **/
            static DPrimitive_gco_2_gco_gco * make_nth_pm(obj<AAllocator> mm,
                                                          StringTable * stbl);

            /** create primitive: create cons cell **/
            static DPrimitive_gco_2_gco_gco * make_cons_pm(obj<AAllocator> mm,
                                                           StringTable * stbl);

            /** create primitive: set first member of cons cell **/
            static DPrimitive_gco_2_gco_gco * make_set_car_pm(obj<AAllocator> mm,
                                                              StringTable * stbl);

            /** create primitive for creating a dictionary instance **/
            static DPrimitive_gco_0 * make_dict_make_pm(obj<AAllocator> mm,
                                                        StringTable * stbl);

            /** create primitive for creating a dictionary instance **/
            static DPrimitive_gco_2_dict_string * make_dict_lookup_pm(obj<AAllocator> mm,
                                                                      StringTable * stbl);

            /** create primitive that upserts a key,value pair into a dictionary **/
            static DPrimitive_gco_3_dict_string_gco * make_dict_upsert_pm(obj<AAllocator> mm,
                                                                          StringTable * stbl);

            /** create primitive: get fixed number of args for function **/
            static DPrimitive_gco_1_gco * make_fn_n_args_pm(obj<AAllocator> mm,
                                                            StringTable * stbl);
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end ObjectPrimitives.hpp */
