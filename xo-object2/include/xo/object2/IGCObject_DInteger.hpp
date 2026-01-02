/** @file IGCObject_DInteger.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <xo/gc/Collector.hpp>
#include "xo/alloc2/alloc/AAllocator.hpp"
#include <xo/gc/detail/AGCObject.hpp>
#include <xo/gc/detail/IGCObject_Xfer.hpp>
#include "DInteger.hpp"

namespace xo {
    namespace scm { struct IGCObject_DInteger; }

    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AGCObject,
                                   xo::scm::DInteger>
        {
            using ImplType = xo::mm::IGCObject_Xfer
                <xo::scm::DInteger,
                 xo::scm::IGCObject_DInteger>;
        };
    }

    namespace scm {
        /* changes here coordinate with:
         *  IGCObject_Xfer
         */
        struct IGCObject_DInteger {
        public:
            using AAllocator = xo::mm::AAllocator;
            using ACollector = xo::mm::ACollector;
            using size_type = std::size_t;

            static size_type shallow_size(const DInteger & d) noexcept;
            static DInteger * shallow_copy(const DInteger & d, obj<AAllocator> mm) noexcept;
            static size_type forward_children(DInteger & d, obj<ACollector> gc) noexcept;
        };
    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DInteger.hpp */
