/** @file DX1CollectorIterator.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "alloc/AllocInfo.hpp"
#include "gc/generation.hpp"
#include "arena/DArenaIterator.hpp"
#include "cmpresult.hpp"

namespace xo {
    namespace mm {
        struct DX1Collector;

        /** @class DX1CollectorIterator
         *  @brief Representation for alloc iterator over X1 collector
         *
         *  Will iterate across all allocs in all generations
         **/
        struct DX1CollectorIterator {
            DX1CollectorIterator() = default;
            DX1CollectorIterator(const DX1Collector * gc,
                                 generation gen_ix,
                                 generation gen_hi,
                                 DArenaIterator arena_ix,
                                 DArenaIterator arena_hi);

            /** Invalid iterator. Does not compare equal to anything, including itself **/
            static DX1CollectorIterator invalid() { return DX1CollectorIterator(); }
            /** Create iterator pointing to the beginning of @p gc.
             *  Iterator cannot modify payload memory
             **/
            static DX1CollectorIterator begin(DX1Collector * gc);
            /** Create iterator pointing to the end of @p gc.
             *  Iterator cannot modify payload memory.
             **/
            static DX1CollectorIterator end(DX1Collector * gc);

            /** true if iterator is invalid. invalid iterators are not comparable **/
            bool is_valid() const noexcept { return (gc_ != nullptr); }
            bool is_invalid() const noexcept { return !is_valid(); }

            generation gen_ix() const { return gen_ix_; }
            generation gen_hi() const { return gen_hi_; }
            DArenaIterator arena_ix() const { return arena_ix_; }
            DArenaIterator arena_hi() const { return arena_hi_; }

            /** fetch contents at current iterator position **/
            AllocInfo deref() const noexcept;
            /** compare two iterators. To be comparable,
             * iterators must refer to the same collector
             **/
            cmpresult compare(const DX1CollectorIterator & other) const noexcept;
            /** advance iterator to next allocation **/
            void next() noexcept;

            /** for *ix synonym for ix.deref() **/
            AllocInfo operator*() const noexcept { return this->deref(); }

        private:
            /** if non-empty, normalize to state with arena_ix_ != arena_hi_ **/
            void normalize() noexcept;

        private:
            /** Iterator visits allocations from this collector **/
            const DX1Collector * gc_ = nullptr;
            /** Iterating over generations in [@p gen_ix_, @p gen_hi_).
             *  Current position is within arena for @p gen_ix_ to-space,
             *  Provided @p gen_ix_ < @p gen_hi_
             **/
            generation gen_ix_;
            generation gen_hi_;
            /** Iterating over allocs in [@p arena_ix_, @p arena_hi_).
             *  Current position is at @p arena_ix_
             **/
            DArenaIterator arena_ix_;
            DArenaIterator arena_hi_;
        };

        inline bool
        operator==(const DX1CollectorIterator & x, const DX1CollectorIterator & y) {
            return x.compare(y).is_equal();
        }

        inline bool
        operator!=(const DX1CollectorIterator & x, const DX1CollectorIterator & y) {
            return !x.compare(y).is_equal();
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end DX1CollectorIterator.hpp */
