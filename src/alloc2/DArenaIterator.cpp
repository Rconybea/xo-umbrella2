/** @file DArenaIterator.cpp
*
 *  @author Roland Conybeare, Dec 2025
 **/

#include "arena/DArenaIterator.hpp"
#include "arena/DArena.hpp"
#include <xo/indentlog/scope.hpp>
#include <xo/indentlog/print/tag.hpp>
#include <cassert>

namespace xo {
    using std::byte;

    namespace mm {
        DArenaIterator
        DArenaIterator::begin(const DArena * arena)
        {
            constexpr bool c_debug_flag = false;
            scope log(XO_DEBUG(c_debug_flag));

            assert(arena);

            if (arena->config_.store_header_flag_ == false) {
                arena->capture_error(error::alloc_iterator_not_supported);

                return DArenaIterator::invalid();
            }

            byte *       begin_byte = arena->lo_;
            AllocHeader * begin_hdr = (AllocHeader *)begin_byte;

            log && log(xtag("begin_hdr", begin_hdr));

            return DArenaIterator(arena, begin_hdr);
        }

        DArenaIterator
        DArenaIterator::end(const DArena * arena)
        {
            constexpr bool c_debug_flag = false;
            scope log(XO_DEBUG(c_debug_flag));

            assert(arena);

            if (arena->config_.store_header_flag_ == false) {
                arena->capture_error(error::alloc_iterator_not_supported);

                return DArenaIterator::invalid();
            }

            byte *       end_byte = arena->free_;
            AllocHeader * end_hdr = (AllocHeader *)end_byte;

            log && log(xtag("end_hdr", end_hdr));

            return DArenaIterator(arena, end_hdr);
        }

        AllocInfo
        DArenaIterator::deref() const noexcept
        {
            constexpr bool c_debug_flag = false;
            scope log(XO_DEBUG(c_debug_flag));

            bool contains_flag = arena_->contains(this->pos_as_byte());
            bool   bounds_flag = (this->pos_as_byte() < arena_->free_);

            log && log(xtag("contains_flag", contains_flag),
                       xtag("bounds_flag", bounds_flag));

            if (!contains_flag || !bounds_flag) {
                arena_->capture_error(error::alloc_iterator_deref);

                return AllocInfo::error_invalid_iterator(&(arena_->config_.header_));
            }

            /* iterator points to beginning of header.
             * memory given to application start immediately followed header
             */

            byte * mem = (byte *)(pos_ + 1);

            return arena_->alloc_info(mem);
        }

        cmpresult
        DArenaIterator::compare(const DArenaIterator & other_ix) const noexcept
        {
            if (is_invalid() || (arena_ != other_ix.arena_))
                return cmpresult::incomparable();

            if (pos_ < other_ix.pos_) {
                return cmpresult::lesser();
            } else if(pos_ == other_ix.pos_) {
                return cmpresult::equal();
            } else {
                return cmpresult::greater();
            }
        }

        void
        DArenaIterator::next() noexcept
        {
            constexpr bool c_debug_flag = false;
            scope log(XO_DEBUG(c_debug_flag));

            bool contains_flag = arena_->contains(this->pos_as_byte());
            bool   bounds_flag = (this->pos_as_byte() < arena_->free_);

            log && log(xtag("contains_flag", contains_flag),
                       xtag("bounds_flag", bounds_flag));

            if (!contains_flag || !bounds_flag) {
                arena_->capture_error(error::alloc_iterator_next);
                return;
            }

            size_t   mem_z = arena_->config_.header_.size(*pos_);
            size_t guard_z = arena_->config_.header_.guard_z_;

            byte * next_as_byte = ((byte *)pos_ + sizeof(AllocHeader) + mem_z + guard_z);
            /* next == ix.arena_free_ --> iterator is at end of allocator */
            assert(next_as_byte <= arena_->free_);

            AllocHeader * next = (AllocHeader *)next_as_byte;

            this->pos_ = next;
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end DArenaIterator.cpp */
