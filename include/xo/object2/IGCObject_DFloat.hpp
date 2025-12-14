/** @file IGCObject_DFloat.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "xo/alloc2/AAllocator.hpp"
#include <xo/alloc2/gc/AGCObject.hpp>
#include <xo/alloc2/gc/IGCObject_Xfer.hpp>
#include "DFloat.hpp"

namespace xo {
    namespace scm {
        /* changes here coordinate with:
         *  IGCObject_Xfer
         */
        struct IGCObject_DFloat {
        public:
            using AAllocator = xo::mm::AAllocator;
            using size_type = std::size_t;

            static size_type shallow_size(const DFloat & d) noexcept;
            static DFloat * shallow_copy(const DFloat & d, obj<AAllocator> mm) noexcept;
            static size_type forward_children(DFloat & d) noexcept;
        };
    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DFloat.hpp */
