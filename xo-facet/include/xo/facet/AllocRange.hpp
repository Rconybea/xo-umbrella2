/** @file AllocRange.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "AllocIterator.hpp"

namespace xo {
    namespace mm {
        /** @class AllocRange
         *  @brief Provide range iteration over an @ref AAllcator
         *
         *  Return value type for @ref AAllocator::alloc_range
         **/
        struct AllocRange {
        public:
            using repr_type = std::pair<obj<AAllocIterator>, obj<AAllocIterator>>;

        public:
            AllocRange() = default;
            explicit AllocRange(repr_type range) : range_{std::move(range)} {}

            obj<AAllocIterator> begin() const { return range_.first; }
            obj<AAllocIterator>   end() const { return range_.second; }

            /** state: {begin,end} pair of alloc iterators **/
            repr_type range_;
        };
    } /*namsepace mm*/
} /*namespace xo*/

/* end AllocRange.hpp */
