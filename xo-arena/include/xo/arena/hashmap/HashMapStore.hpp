/** @file HashMapStore.hpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include <xo/arena/hashmap/ControlGroup.hpp>
#include <xo/arena/hashmap/DArenaHashMapUtil.hpp>
#include <xo/arena/DArenaVector.hpp>
#include <cstdint>
#include <cassert>

namespace xo {
    namespace map {
        namespace detail {
            /**
             *  @code
             *
             *    lo stub                           hi stub
             *    >    <                            >    <
             *    .    .                       wrap .    .
             *    .    .                       >    <    .
             *    .    .                       .    .    .
             *     xxxx 0000 1111 2222 ... ZZZZ 0000 xxxx
             *     ^    ^                  ^    ^
             *     0    16                      N (= 16*NG)
             *          ^                  ^
             *          group(0)           group(NG-1)
             *
             *  @endcode
             **/
            template <typename Key,
                      typename Value>
            struct HashMapStore : DArenaHashMapUtil {
            public:
                using value_type = std::pair<const Key, Value>;
                using storage_type = std::pair<Key, Value>;
                using group_type = detail::ControlGroup;
                using control_vector_type = xo::mm::DArenaVector<uint8_t>;
                using slot_vector_type = xo::mm::DArenaVector<storage_type>;
                using MemorySizeVisitor = xo::mm::MemorySizeVisitor;
                using MemorySizeInfo = xo::mm::MemorySizeInfo;

            public:
                /** group_exp2: number of groups {x, 2^x} **/
                explicit HashMapStore(const std::string & name,
                                      const std::pair<size_type,
                                                      size_type> & group_exp2)
                : size_{0},
                  n_group_exponent_{group_exp2.first},
                  n_group_{group_exp2.second},
                  n_slot_{group_exp2.second * c_group_size},
                  control_{control_vector_type::map
                    (xo::mm::ArenaConfig{
                        .name_ = name + "-ctl",
                        .size_ = control_size(n_slot_),
                        .store_header_flag_ = false})},
                  slots_{slot_vector_type::map
                    (xo::mm::ArenaConfig{
                        .name_ = name + "-slots",
                        .size_ = n_slot_ * sizeof(value_type),
                        .store_header_flag_ = false})}
                {
                    /* here: arenas have allocated address range, but no committed memory yet */

                    this->_init();
                }

                static_assert(sizeof(storage_type) == sizeof(value_type));
                static_assert(alignof(storage_type) == alignof(value_type));

                static value_type * to_value(storage_type * x) {
                    return reinterpret_cast<value_type *>(x);
                }

                static const value_type * to_value(const storage_type * x) {
                    return reinterpret_cast<const value_type *>(x);
                }

                size_type empty() const noexcept { return size_ == 0; }
                /** current hash tabel capacity.
                 *  Promise: always a power of 2
                 **/
                size_type capacity() const noexcept { return n_group_ * c_group_size; }
                float load_factor() const noexcept { return size_ / static_cast<float>(n_slot_); }

                void visit_pools(const MemorySizeVisitor & visitor) const {
                    // complexity here in service of HashMapStore-specific value for MemorySizeInfo.used

                    MemorySizeInfo ctl_info;
                    MemorySizeInfo slot_info;

                    control_.visit_pools([&ctl_info](const auto & x) { ctl_info = x; });
                    slots_.visit_pools([&slot_info](const auto & x) { slot_info = x; });

                    // control: 1 byte per (key,value) pair
                    ctl_info.used_ = size_;
                    slot_info.used_ = size_ * sizeof(value_type);

                    visitor(ctl_info);
                    visitor(slot_info);
                }

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
                group_type _load_group(size_type ix) const {
                    return group_type(&(control_[ix + c_control_stub]));
                }

                /** count consecutive non-empty control bytes in range
                 *    [ix - g1, ..., ix, .., ix + g1)
                 *  for 16-byte intervals that include position logical position ix.
                 *  (actually shifted right by 16 to make room for lo_stub
                 **/
                bool _needs_tombstone(size_type ix) const {
                    /* group before ix **/
                    size_type g0_ix;

                    if (ix >= c_group_size) {
                        g0_ix = ix - c_group_size + 1;
                    } else {
                        /**
                         *  Near beginning of control-group array,
                         *  use the duplicate group at the end.
                         *
                         *    ix                      ix+N
                         *    v                       v
                         *   0000 1111 2222 ... ZZZZ 0000
                         *                        < g0 >
                         **/

                        size_type N = this->capacity();

                        // near beginning of array, use wrap group at the end
                        g0_ix = ix - c_group_size + 1 + N;
                    }

                    /**
                     *      <----- g0 ----->
                     *      1111222233334444
                     *                     1111222233334444
                     *                     <------ g1 ---->
                     *
                     *                     ^
                     *                    ix (counted twice)
                     **/

                    /* g0: 16 bytes before ix including ix itself */
                    auto g0 = this->_load_group(g0_ix);
                    /* g1: 16 bytes after ix including ix itself */
                    auto g1 = this->_load_group(ix);

                    /* 1 bits tell which members of groups g0,g1 are empty,
                     * byte j goes into bit j -> lo address in *least significant* bit.
                     */
                    auto before = g0.empty_matches();
                    auto after = g1.empty_matches();

                    auto run_left = std::countl_zero(before);
                    auto run_right = std::countr_zero(after);

                    /* run_left, run_right count position ix twice -> subtract 1 */
                    return (run_left + run_right) >= c_group_size + 1;
                } /*_need_tombstone*/

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
