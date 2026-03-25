/** @file GcPrimitives.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

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
            /** create primitive: request collection **/
            static DPrimitive_gco_1_gco * make_request_gc_pm(obj<AAllocator> mm,
                                                             StringTable * stbl);
        };

    } /*namespace scm*/
} /*namespace xo*/


/* end GcPrimitives.hpp */
