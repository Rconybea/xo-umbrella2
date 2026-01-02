/** @file IGCObject_DFloat.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <xo/alloc2/Allocator.hpp>
#include <xo/gc/Collector.hpp>
#include <xo/gc/detail/AGCObject.hpp>
#include <xo/gc/detail/IGCObject_Xfer.hpp>
#include "DFloat.hpp"

namespace xo {
    namespace scm { struct IGCObject_DFloat; }

    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AGCObject,
                                   xo::scm::DFloat>
        {
            using ImplType = xo::mm::IGCObject_Xfer
                <xo::scm::DFloat,
                 xo::scm::IGCObject_DFloat>;
        };
    }

    namespace scm {
        /* changes here coordinate with:
         *  IGCObject_Xfer
         */
        struct IGCObject_DFloat {
        public:
            using AAllocator = xo::mm::AAllocator;
            using ACollector = xo::mm::ACollector;
            using size_type = std::size_t;

            static size_type shallow_size(const DFloat & d) noexcept;
            static DFloat * shallow_copy(const DFloat & d, obj<AAllocator> mm) noexcept;
            static size_type forward_children(DFloat & d, obj<ACollector> gc) noexcept;
        };
    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DFloat.hpp */
