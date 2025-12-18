/** @file IAllocIterator_DX1Collector.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "alloc/IAllocIterator_Xfer.hpp"
#include "gc/DX1CollectorIterator.hpp"

namespace xo {
    namespace mm { struct IAllocIterator_DX1CollectorIterator; }

    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AAllocIterator,
                                   xo::mm::DX1CollectorIterator> {
            using ImplType = xo::mm::IAllocIterator_Xfer<xo::mm::DX1CollectorIterator,
                                                         xo::mm::IAllocIterator_DX1CollectorIterator>;
        };
    }

    namespace mm {
        /** @class IAllocIterator_DX1Collector
         *  @brief alloc iteration for the DX1Collector allocator
         **/
        struct IAllocIterator_DX1CollectorIterator {
            static AllocInfo deref(const DX1CollectorIterator &) noexcept;
            static cmpresult compare(const DX1CollectorIterator &,
                                     const obj<AAllocIterator> & other) noexcept;
            static void next(DX1CollectorIterator &) noexcept;
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end IAllocIterator_DX1Collector.hpp */
