/** @file DArenaIterator.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "AllocInfo.hpp"
#include "AllocHeader.hpp"
#include "cmpresult.hpp"

namespace xo {
    namespace mm {
        struct DArena;

        /** @class DArenaIterator
         *  @brief Representation for alloc iterator over arena
         *
         *  Map showing an arena allocation:
         *
         *  @verbatim
         *
         *                    <-------------z1--------------->
         *           < guard ><  hz  ><     req_z     >< dz  >< guard >
         *
         *           +++++++++0000zzzz@@@@@@@@@@@@@@@@@ppppppp+++++++++
         *
         *                    ^       ^                                ^
         *                    header  mem                              header
         *                    ^                                        (next alloc)
         *                    DArenaIterator::pos_
         *
         *     guard  [+] guard before+after each allocation, for simple sanitize checks
         *    header  [0] alloc header (non-size bits)
         *            [z] alloc header (size bits)
         *       mem  [@] app-requested memory, including padding [p]
         *        dz  [p] padding (to uintptr_t alignment.  req_z+dz recorded in header)
         *     free_      DArena::free_ just after guard bytes for last allocation
         *
         *  @endverbatim
         **/
        struct DArenaIterator {
            /** @defgroup mm-arenaiterator-ctors DArenaIterator instance vars **/
            ///@{
            DArenaIterator() = default;
            DArenaIterator(const DArena * arena,
                           AllocHeader * pos) : arena_{arena},
                                                pos_{pos} {}

            /** Create iterator in invalid state **/
            static DArenaIterator invalid() { return DArenaIterator(); }

            /** Create iterator pointing to the beginning of @p arena
             *  Iterator cannot modify memory, but can capture
             *  an iterator error in @p *arena
             **/
            static DArenaIterator begin(const DArena * arena);
            /** Create iterator pointing to the end of @p arena
             *  Iterator cannot modify memory, but can capture
             *  an iterator error in @p *arena
             **/
            static DArenaIterator   end(const DArena * arena);
            ///@}

            /** @defgroup mm-arenaiterator-methods DArenaIterator methods **/
            ///@{
            /** Address of allocation header for beginning of alloc range in @p arena **/
            static AllocHeader * begin_header(const DArena * arena);
            /** Address of allocation header for end of alloc range.
             *  This is the address of header for _next_ allocation in @p arena
             *  i.e. free pointer
             **/
            static AllocHeader *   end_header(const DArena * arena);

            /** A valid iterator can be compared, at least with itself
             *  It can be dereferenced if is also non-empty
             **/
            bool is_valid() const noexcept { return (arena_ != nullptr) && (pos_ != nullptr); }
            /** An invalid (or sentinel) iterator is incomparable with all
             *  iterators including itself
             **/
            bool is_invalid() const noexcept { return !is_valid(); }

            /** fetch contents at current iterator position **/
            AllocInfo deref() const noexcept;
            /** compare two iterators. To be comparable,
             *  iterators must refer to the same arena
             **/
            cmpresult compare(const DArenaIterator & other) const noexcept;
            /** advance iterator to next allocation **/
            void next() noexcept;

            /** cast iterator position to byte* */
            std::byte * pos_as_byte() const { return (std::byte *)pos_; }

            /** *ix synonym for ix.deref() **/
            AllocInfo operator*() const noexcept { return this->deref(); }
            /** ++ix synonym for ix.next() **/
            DArenaIterator & operator++() noexcept { this->next(); return *this; }
            ///@}

            /** @defgroup mm-arenaiterator-instance-vars **/
            ///@{
            /** iterator visits allocations from this arena **/
            const DArena * arena_ = nullptr;
            /** current iterator position **/
            AllocHeader * pos_ = nullptr;
            ///@}
        };

        inline bool
        operator==(const DArenaIterator & x,
                   const DArenaIterator & y)
        {
            return x.compare(y).is_equal();
        }

        inline bool
        operator!=(const DArenaIterator & x,
                   const DArenaIterator & y)
        {
            return !x.compare(y).is_equal();
        }
    } /*namespace mm*/
} /*namespace xo*/

/* end DArenaIterator.hpp */
