/** @file DArenaHashMapIterator.hpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "hashmap/DArenaHashMapUtil.hpp"

namespace xo {
    namespace mm {
        namespace detail {

            template <typename Key,
                      typename Value>
            struct DArenaHashMapIterator : public DArenaHashMapUtil {
                using value_type = std::pair<const Key, Value>;

            public:
                DArenaHashMapIterator(uint8_t * c, value_type * p)
                    : ctrl_{c}, pos_{p} {}

                value_type & operator*() const { return *pos_; }
                value_type * operator->() const { return pos_; }

                /** true iff iterator at sentinel position (not dereferencable state !) **/
                bool _at_slot_sentinel() const { return is_sentinel(*ctrl_) && (*ctrl_ != c_iterator_bookend); }

                bool operator==(const DArenaHashMapIterator & x) const {
                    return this->pos_ == x.pos_;
                }

                bool operator!=(const DArenaHashMapIterator & x) const {
                    return this->pos_ != x.pos_;
                }

                DArenaHashMapIterator & operator++() {
                    do {
                        ++ctrl_;
                        ++pos_;

                        /** end condition: iterator ends at last non-wrapped position.
                         *  relyin on bookend sentinel values at known offset from 'wrap' section
                         *
                         *                                        ctrl_       ctrl_ + c_group_size
                         *                                            |       |
                         *                                            v       v
                         *   <----------------- control_size(n_slot) ---------------->
                         *   <-stub-> <----------- n_slot ----------> <group> <-stub->
                         *  +--------+-------------------------------+-------+--------+
                         *  | 0xF0   | empty / data / tombstone      | wrap  | 0xF0   |
                         *  +--------+-------------------------------+-------+--------+
                         **/
                    } while (is_sentinel(*ctrl_)
                             && (*(ctrl_ + c_group_size) != c_iterator_bookend));

                    return *this;
                }

                DArenaHashMapIterator & operator--() {
                    /* simpler than forward iteration, since bookend immediately
                     * precedes control byte for first slot
                     */
                    do {
                        --ctrl_;
                        --pos_;
                    } while (is_sentinel(*ctrl_)
                             && (*ctrl_ != c_iterator_bookend));

                    return *this;
                }

            private:
                uint8_t * ctrl_ = nullptr;
                value_type * pos_ = nullptr;
            };

        }
    } /*namespace mm*/
} /*namespace xo*/

/* end DArenaHashMapIterator.hpp */
