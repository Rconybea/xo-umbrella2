/** @file DArenaHashMapIterator.hpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "HashMapStore.hpp"
#include <xo/arena/hashmap/DArenaHashMapUtil.hpp>

namespace xo {
    namespace map {
        namespace detail {

            template <typename Key,
                      typename Value>
            struct DArenaHashMapIterator : public DArenaHashMapUtil {
                using store_type = HashMapStore<Key, Value>;
                using value_type = std::pair<const Key, Value>;
                using storage_type = std::pair<Key, Value>;

            public:
                DArenaHashMapIterator(uint8_t * c, storage_type * p)
                    : ctrl_{c}, pos_{p} {}

                value_type & operator*() const { return *store_type::to_value(pos_); }
                value_type * operator->() const { return store_type::to_value(pos_); }

                uint8_t * _ctrl() const { return ctrl_; }
                storage_type * _pos() const { return pos_; }

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
                        ++(this->ctrl_);
                        ++(this->pos_);

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
                        --(this->ctrl_);
                        --(this->pos_);
                    } while (is_sentinel(*ctrl_)
                             && (*ctrl_ != c_iterator_bookend));

                    return *this;
                }

            private:
                uint8_t * ctrl_ = nullptr;
                /** key,value pair under current iterator position **/
                storage_type * pos_ = nullptr;
            };

            template <typename Key,
                      typename Value>
            struct DArenaHashMapConstIterator : public DArenaHashMapUtil {
                using value_type = std::pair<const Key, Value>;
                using storage_type = std::pair<Key, Value>;
                using store_type = HashMapStore<Key, Value>;

            public:
                DArenaHashMapConstIterator(const uint8_t * c, const storage_type * p)
                    : ctrl_{c}, pos_{p} {}

                const value_type & operator*() const { return *store_type::to_value(pos_); }
                const value_type * operator->() const { return store_type::to_value(pos_); }

                const uint8_t * _ctrl() const { return ctrl_; }
                const storage_type * _pos() const { return pos_; }

                /** true iff iterator at sentinel position (not dereferencable state !) **/
                bool _at_slot_sentinel() const {
                    return is_sentinel(*ctrl_) && (*ctrl_ != c_iterator_bookend);
                }

                bool operator==(const DArenaHashMapConstIterator & x) const {
                    return this->pos_ == x.pos_;
                }

                bool operator!=(const DArenaHashMapConstIterator & x) const {
                    return this->pos_ != x.pos_;
                }

                DArenaHashMapConstIterator & operator++() {
                    do {
                        ++(this->ctrl_);
                        ++(this->pos_);

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

                DArenaHashMapConstIterator & operator--() {
                    /* simpler than forward iteration, since bookend immediately
                     * precedes control byte for first slot
                     */
                    do {
                        --(this->ctrl_);
                        --(this->pos_);
                    } while (is_sentinel(*ctrl_)
                             && (*ctrl_ != c_iterator_bookend));

                    return *this;
                }

            private:
                /** control byte under current iterator position **/
                const uint8_t * ctrl_ = nullptr;
                /** key,value pair under current iterator position **/
                const storage_type * pos_ = nullptr;
            }; /*DArenaHashMapConstIterator*/
        } /*namespace detail*/
    } /*namespace map*/
} /*namespace xo*/

/* end DArenaHashMapIterator.hpp */
