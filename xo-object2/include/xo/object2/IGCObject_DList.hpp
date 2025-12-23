/** @file IGCObject_DList.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <xo/alloc2/alloc/AAllocator.hpp>
#include <xo/alloc2/alloc/RAllocator.hpp>
#include <xo/gc/Collector.hpp>
#include <xo/gc/AGCObject.hpp>
#include <xo/gc/detail/IGCObject_Xfer.hpp>
#include "DList.hpp"

namespace xo {
    namespace scm {
        /* changes here coordinate with:
         *  IGCObject_XFer
         */
        struct IGCObject_DList {
        public:
            using AAllocator = xo::mm::AAllocator;
            using ACollector = xo::mm::ACollector;
            using size_type = std::size_t;

            static size_type shallow_size(const DList & d) noexcept;
            static DList * shallow_copy(const DList & d, obj<AAllocator> mm) noexcept;
            static size_type forward_children(DList & d, obj<ACollector> gc) noexcept;
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DList.hpp */
