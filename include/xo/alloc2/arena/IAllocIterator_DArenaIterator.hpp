/** @file IAllocIterator_DArenaIterator.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "alloc/IAllocIterator_Xfer.hpp"
#include "arena/DArenaIterator.hpp"

namespace xo {
    namespace mm { struct IAllocIterator_DArenaIterator; }

    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AAllocIterator,
                                   xo::mm::DArenaIterator> {
            using ImplType = xo::mm::IAllocIterator_Xfer<xo::mm::DArenaIterator,
                                                         xo::mm::IAllocIterator_DArenaIterator>;
        };
    }

    namespace mm {
        /** @class IAllocIterator_DArena
         *  @brief alloc iteration for the DArena allocator
         **/
        struct IAllocIterator_DArenaIterator {
            static AllocInfo deref(const DArenaIterator &) noexcept;
            static cmpresult compare(const DArenaIterator &,
                                     const obj<AAllocIterator> & other) noexcept;
            static void next(DArenaIterator &) noexcept;
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end IAllocIterator_DArenaIterator.cpp */
