/** @file GcPrimitives.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

#include "Primitive_gco_0.hpp"
#include "Primitive_gco_1_gco.hpp"

namespace xo {
    namespace scm {

        /** @rbief primitives centered on gc/ data structures.
         *  (i.e. X1Collector)
         **/
        class GcPrimitives {
        public:
            using AAllocator = xo::mm::AAllocator;

        public:
            /** create primitive: report gc statistics **/
            static DPrimitive_gco_0 * make_report_gc_statistics_pm(obj<AAllocator> mm,
                                                                   StringTable * stbl);

            /** create primitive: report gc object-type statistics **/
            static DPrimitive_gco_0 * make_report_gc_object_types_pm(obj<AAllocator> mm,
                                                                     StringTable * stbl);

            /** create primitive: report gc object-age statistics **/
            static DPrimitive_gco_0 * make_report_gc_object_ages_pm(obj<AAllocator> mm,
                                                                    StringTable * stbl);

            /** create primitive: report gc location of a value **/
            static DPrimitive_gco_1_gco * make_gc_location_of_pm(obj<AAllocator> mm,
                                                                 StringTable * stbl);

            /** create primitive: request collection **/
            static DPrimitive_gco_1_gco * make_request_gc_pm(obj<AAllocator> mm,
                                                             StringTable * stbl);
        };

    } /*namespace scm*/
} /*namespace xo*/


/* end GcPrimitives.hpp */
