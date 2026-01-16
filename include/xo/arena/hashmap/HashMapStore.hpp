/** @file HashMapStore.hpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "hashmap/DArenaHashMapUtil.hpp"
#include "hashmap/ControlGroup.hpp"

namespace xo {
    namespace map {
        namespace detail {
            template <typename Key,
                      typename Value>
            struct HashMapStore : DArenaHashMapUtil {
            public:
                using value_type = std::pair<const Key, Value>;
                using group_type = detail::ControlGroup;
                using control_vector_type = xo::mm::DArenaVector<uint8_t>;
                using slot_vector_type = xo::mm::DArenaVector<value_type>;

            public:
                /** group_exp2: number of groups {x, 2^x} **/
                explicit HashMapStore(const std::string & name,
                                      const std::pair<size_type,
                                                      size_type> & group_exp2)
                : size_{0},
                  n_group_exponent_{group_exp2.first},
                  n_group_{group_exp2.second},
                  n_slot_{group_exp2.second * c_group_size},
                  control_{control_vector_type::map(xo::mm::ArenaConfig{.name_ = name, .size_ = control_size(n_slot_)})},
                  slots_{slot_vector_type::map(xo::mm::ArenaConfig{.name_ = name, .size_ = n_slot_ * sizeof(value_type)})}
                {
                    /* here: arenas have allocated address range, but no committed memory yet */

                    this->_init();
                }

                size_type empty() const noexcept { return size_ == 0; }
                size_type capacity() const noexcept { return n_group_ * c_group_size; }
                float load_factor() const noexcept { return size_ / static_cast<float>(n_slot_); }

                void resize_from_empty(const std::pair<size_type,
                                                       size_type> & group_exp2)
                {
                    assert(size_ == 0);

                    this->n_group_exponent_ = group_exp2.first;
                    this->n_group_ = group_exp2.second;
                    this->n_slot_ = group_exp2.second * c_group_size;

                    this->_init();
                }

                void clear() {
                    /* remark: discontinuity in the sense that we lose n_group_ = 2 ^ n_group_epxonent_
                     *
                     * juice may not be worth the squeeze here,
                     * since DArena doesn't yet (Jan 2026) unmap on clear
                     */

                    this->size_ = 0;
                    this->n_group_exponent_ = 0;
                    this->n_group_ = 0;
                    this->n_slot_ = 0;
                    this->control_.resize(0);
                    this->slots_.resize(0);
                }

            public:
                void _init() {
                    this->control_.resize(control_size(n_slot_));

                    /* front stub: iterator bookend */
                    std::fill(this->control_.begin(),
                              this->control_.begin() + c_control_stub,
                              c_iterator_bookend);

                    /* all slots marked empty initially */
                    std::fill(this->control_.begin() + c_control_stub,
                              this->control_.end() - c_control_stub,
                              c_empty_slot);

                    /* end stub: iterator bookend */
                    std::fill(this->control_.end() - c_control_stub,
                              this->control_.end(),
                              c_iterator_bookend);

                    this->slots_.resize(n_slot_);
                }

                /** load control group for slot range [ix .. ix+c_group_size) **/
                group_type _load_group(size_type ix) {
                    return group_type(&(control_[ix + c_control_stub]));
                }

                /** update control group for slot number @p ix, replace with @p h2 **/
                void _update_control(size_type ix, uint8_t h2) {
                    this->control_[ix + c_control_stub] = h2;

                    if (ix < c_group_size) {
                        size_type N = this->capacity();

                        // refresh end-of-array copy
                        std::memcpy(&(control_[N + c_control_stub]),
                                    &(control_[c_control_stub]),
                                    c_group_size);
                    }
                }

            public:
                /** number of pairs in this table **/
                size_type size_ = 0;
                /** base-2 logarithm of n_group_ **/
                size_type n_group_exponent_ = 0;
                /** table has capacity for this number of groups.
                 *  always an exact power of two.
                 *  number of slots is n_group_ * c_group_size
                 **/
                size_type n_group_ = (1 << n_group_exponent_);
                /** table has capacity for this number of {key,value} pairs **/
                size_type n_slot_ = n_group_ * c_group_size;
                /** control_[] partitioned into groups of
                 *  c_group_size (16) consecutive elements
                 **/
                control_vector_type control_;
                /** slots_[] holds {key,value} pairs **/
                slot_vector_type slots_;
            };
        }
    } /*namespace map*/
} /*namespace xo*/

/* end HashMapStore.hpp */
