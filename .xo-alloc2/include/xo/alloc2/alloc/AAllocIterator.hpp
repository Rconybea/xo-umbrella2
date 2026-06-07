/** @file AAllocIterator.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "AllocInfo.hpp"
#include "cmpresult.hpp"
#include "typeseq.hpp"
#include <xo/facet/obj.hpp>

namespace xo {
    namespace mm {
        using Copaque = const void *;
        using Opaque = void *;

        /** @class AAllocIterator
         *  @brief Abstract facet for iterating over allocs
         *
         *  Iterator refers to an AllocInfo instance
         *  Only supporting forward-allocator.
         **/
        struct AAllocIterator {
            using obj_AAllocIterator = xo::facet::obj<AAllocIterator>;
            using typeseq = xo::reflect::typeseq;

            /** @defgroup mm-allociterator-methods AllocIterator methods **/
            ///@{
            /** RTTI: unique id# for actual runtime *data* representation **/
            virtual typeseq _typeseq() const noexcept = 0;
            /** retrieve AllocInfo for current iterator position
             **/
            virtual AllocInfo deref(Copaque d) const noexcept = 0;
            /** compare alloc iterators @p d and @p other **/
            virtual cmpresult compare(Copaque d,
                                      const obj_AAllocIterator & other) const noexcept = 0;
            /** advance iterator to next position **/
            virtual void next(Opaque d) const noexcept = 0;
            ///@}
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end AAllocIterator.hpp */
