/** @file DArenaHashMap.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "DArenaVector.hpp"
#include <xo/indentlog/scope.hpp>
#include <algorithm>
#include <array>
#include <utility>
#include <cstring>

namespace xo {
    struct verify_policy {
        static verify_policy log_only() {
            return verify_policy{.flags_ = 0x01};
        }
        static verify_policy throw_only() {
            return verify_policy{.flags_ = 0x02};
        }
        static verify_policy chatty() {
            return verify_policy{.flags_ = 0x03};
        }

        bool is_silent() const noexcept { return flags_ == 0; }
        bool log_flag() const noexcept { return flags_ & 0x01; }
        bool throw_flag() const noexcept { return flags_ & 0x02; }

        template<typename... Tn>
        bool report_error(scope & log, Tn&&... args)
        {
            if (!this->is_silent()) {
                // TODO: consider global arena here for string
                std::string msg = tostr(std::forward<Tn>(args)...);

                if (this->log_flag()) {
                    log.retroactively_enable();
                    log(msg);
                }
                if (this->throw_flag()) {
                    throw std::runtime_error(msg);
                }
            }
            return false;
        }

        const char * c_self_ = "anonymous";
        uint8_t flags_;
    };

    namespace mm {
#ifdef NOT_YET
        enum class insert_error : int32_t {
            /** sentinel **/
            invalid = -1,
            /** not an error **/
            ok,
        };
#endif

        struct DArenaHashMapUtil {
            using size_type = std::size_t;
            using control_type = std::uint8_t;

            /** control: sentinel for empty slot **/
            static constexpr uint8_t c_empty_slot = 0xFF;
            /** control: tombstone for deleted slot **/
            static constexpr uint8_t c_tombstone = 0xFE;

            /** group size **/
            static constexpr size_type c_group_size = 16;

            /** max load factor **/
            static constexpr float c_max_load_factor = 0.875;

            /** find smallest multiple k : k * c_group_size >= n **/
            static size_type lub_group_mult(size_t n) {
                return (n + c_group_size - 1) / c_group_size;
            }

            /** find smallest x such that 2^x >= n. Return {x, 2^x} **/
            static std::pair<size_type, size_type> lub_exp2(size_t n) {
                size_type ngx = 0;
                size_type ng = 1;

                while (ng < n) {
                    ++ngx;
                    ng *= 2;
                }

                return std::make_pair(ngx, ng);;
            }
        };

        namespace detail {
            /** @brief 16x 8-bit control bytes.
             *
             *  Support optimization using SIMD operations
             **/
            struct Group {
                std::array<uint8_t, DArenaHashMapUtil::c_group_size> ctrl_;

                explicit Group(uint8_t * lo) {
                    ::memcpy(ctrl_.data(), lo, DArenaHashMapUtil::c_group_size);
                }

                /** find all exact matches in ctrl_[0..15] for @p h2.
                 *  for each match set corresponding bit in return value.
                 *  Bits {0x1, 0x2, 0x4, ...} set iff exact match on
                 *  {ctrl_[0], ctrl_[1], ctrl_2[], ...} respectively
                 **/
                uint16_t all_matches(uint8_t h2) const {
                    uint16_t retval = 0;
                    uint16_t bit = 1;
                    for (auto xi : ctrl_) {
                        if (xi == h2)
                            retval |= bit;
                        bit = bit << 1;
                    }

                    return retval;
                }

                /** find all empty sentinels in ctrl_[0..15].
                 *  for each empty, set corresponding bit in return value.
                 *  Bits {0x1, 0x2, 0x4, ...} set iff empty spot
                 *  {ctrl_[0], ctrl_[1], ctrl_[2], ...} respectively
                 **/
                uint16_t empty_matches() const {
                    uint16_t retval = 0;
                    uint16_t bit = 1;
                    for (auto xi : ctrl_) {
                        if (xi == DArenaHashMapUtil::c_empty_slot)
                            retval |= bit;
                        bit = bit << 1;
                    }

                    return retval;
                }

#ifdef NOT_YET
                __m128i ctrl;  // 16 bytes loaded via SSE2

                // Find all slots matching h2
                uint16_t Match(uint8_t h2) const {
                    __m128i pattern = _mm_set1_epi8(h2);
                    __m128i result = _mm_cmpeq_epi8(ctrl, pattern);
                    return _mm_movemask_epi8(result);  // 16-bit mask
                }

                // Find all empty slots (0xFF)
                uint16_t MatchEmpty() const {
                    return _mm_movemask_epi8(_mm_cmpeq_epi8(ctrl, _mm_set1_epi8(0xFF)));
                }
#endif
            };
        }

        /** @brief flat hash map of key-value pairs using dedicated DArenas for storage
         *
         *  Replicates (to the extent feasible) std::unordered_map<K,V>
         *
         *  @tparam K key type.
         *  @tparam V value type.
         **/
        template <typename Key,
                  typename Value,
                  typename Hash = std::hash<Key>,
                  typename Equal = std::equal_to<void>>
        struct DArenaHashMap : DArenaHashMapUtil {
        public:
            using size_type = DArenaHashMapUtil::size_type;
            using key_type = Key;
            using mapped_type = Value;
            using value_type = std::pair<const Key, Value>;
            using key_hash = Hash;
            using key_equal = Equal;
            using byte = std::byte;
            using group_type = detail::Group;

            /** create hash map **/
            DArenaHashMap(size_type hint_max_capacity,
                          bool debug_flag = false);
            DArenaHashMap(Hash && hash = Hash(),
                          Equal && eq = Equal(),
                          size_type hint_max_capacity = 0,
                          bool debug_flag = false);

            size_type empty() const noexcept { return size_ == 0; }
            size_type groups() const noexcept { return n_group_; }
            size_type size() const noexcept { return size_; }
            size_type capacity() const noexcept { return n_group_ * c_group_size; }

            float load_factor() const noexcept { return size_ / static_cast<float>(n_slot_); }

            /** insert @p kv_pair into hash map. replaces any previous value
             *  stored under the same key.
             *
             *  Return pair retval with:
             *  reval.first: true if size incremented;
             *  retval.second: address of slots_[p] at which pair inserted/updated
             *
             *  When table is full retval.second will be nullptr,
             *  with error captured in last_error_
             **/
            std::pair<value_type *, bool> try_insert(const std::pair<const Key, Value> & kv_pair);

            bool verify_ok(verify_policy p = verify_policy::throw_only()) const;

        private:
            /** load group abstraction from control bytes starting at @p ix **/
            group_type _load_group(size_type ix) {
                return group_type(&(control_[ix]));
            }

            /** like ctrl_[ix] = h2, but maintain overflow copy
             *  at end of ctrl_[] array
             **/
            void _update_control(size_type ix, uint8_t h2);

        private:
            /** hash function **/
            key_hash hash_;
            /** key equal **/
            key_equal equal_;
            /** number of pairs in this table **/
            size_type size_ = 0;
            /** base-2 logarithm of n_group_ **/
            size_type n_group_exponent_ = 0;
            /** table has capacity for this number of groups. always an exact power of two.
             *  number of slots is n_group_ * c_group_size
             **/
            size_type n_group_ = (1 << n_group_exponent_);
            /** table has capacity for this number of {key,value} pairs **/
            size_type n_slot_ = n_group_ * c_group_size;
            /** control_[] partitioned into groups of c_group_size (16) consecutive elements
             **/
            DArenaVector<uint8_t> control_;
            /** slots_[] holds {key,value} pairs **/
            DArenaVector<value_type> slots_;
            /** true to enable debug logging **/
            bool debug_flag_ = false;
        };

        template <typename Key, typename Value, typename Hash, typename Equal>
        DArenaHashMap<Key, Value, Hash, Equal>::DArenaHashMap(size_type hint_max_capacity,
                                                              bool debug_flag)
        : DArenaHashMap(Hash(), Equal(), hint_max_capacity, debug_flag)
        {
        }

        /* remarks:
         * - control: extra 16 slots for safe wraparound.
         *   last 16 bytes will be copy of first 16 bytes
         */
        template <typename Key, typename Value, typename Hash, typename Equal>
        DArenaHashMap<Key, Value, Hash, Equal>::DArenaHashMap(Hash && hash,
                                                              Equal && eq,
                                                              size_type hint_max_capacity,
                                                              bool debug_flag)
        : hash_{std::move(hash)},
          equal_{std::move(eq)},
          size_{0},
          n_group_exponent_{lub_exp2(lub_group_mult(hint_max_capacity)).first},
          n_group_{lub_exp2(lub_group_mult(hint_max_capacity)).second},
          n_slot_{n_group_ * c_group_size},
          control_{DArenaVector<uint8_t>::map(ArenaConfig{.size_ = n_slot_ + c_group_size})},
          slots_{DArenaVector<value_type>::map(ArenaConfig{.size_ = n_slot_ * sizeof(value_type)})},
          debug_flag_{debug_flag}
        {
            /* invariant: arenas have allocated address range, but no committed memory yet */
            this->control_.resize(n_slot_ + c_group_size);

            /* all slots marked empty initially */
            std::fill(this->control_.begin(), this->control_.end(), c_empty_slot);

            this->slots_.resize(n_slot_);
        }

        template <typename Key, typename Value, typename Hash, typename Equal>
        void
        DArenaHashMap<Key, Value, Hash, Equal>::_update_control(size_type ix, uint8_t h2)
        {
            this->control_[ix] = h2;

            if (ix < c_group_size) {
                size_type N = this->capacity();

                // refresh end-of-array copy
                std::memcpy(&(control_[N]), &(control_[0]), c_group_size);
            }
        }

        template <typename Key, typename Value, typename Hash, typename Equal>
        auto
        DArenaHashMap<Key, Value, Hash, Equal>::try_insert(const std::pair<const Key, Value> & kv_pair) -> std::pair<value_type *, bool>
        {
            size_type h = hash_(kv_pair.first);
            // h1: hi bits: probe sequence
            size_type h1 = h >> 7;
            // h2: lo bits: store in control byte
            uint8_t h2 = h & 0x7f;

            size_type N = this->capacity();

            // same as:
            //   ix = h1 % N
            // since N is power of 2
            size_type ix = h1 & (N - 1);

            // will make series of probes
            for (;;) {
                auto grp = _load_group(ix);

                {
                    // look for matching slot to update
                    uint16_t m = grp.all_matches(h2);

                    // process each match.
                    // matches are encountered in the same order they
                    // appear in ctrl_[]
                    while (m) {
                        // zeroes: #of 0 before least-significant 1 bit
                        int skip = __builtin_ctz(m);
                        size_type slot_ix = (ix + skip) & (N - 1);

                        // invariant: slot_ix in [0 .. N)

                        auto & slot = slots_[slot_ix];

                        if (slot.first == kv_pair.first) {
                            // we have match on existing key;
                            // replace associated value
                            slot.second = kv_pair.second;

                            // false: did not change table size
                            return std::make_pair(&slot, false);
                        }

                        // e.g:
                        //             /-- lowest 1 bit gets cleared
                        //             v
                        //  m   = b01101000
                        //  m-1 = b01100111
                        //  &   = b01100000

                        m &= (m - 1);
                    }
                }

                {
                    // look for empty slot to insert
                    uint16_t e = grp.empty_matches();

                    // process each empty slot
                    if (e) {
                        // check that table is below max load factor (0.875).
                        // Check here so that table can stay at max load factor
                        // indefinitely as long as updates only
                        //
                        if (load_factor() >= c_max_load_factor) {
                            return std::make_pair(nullptr, false);
                        }

                        // zeroes: #of 0 before least significant 1 bit
                        int skip = __builtin_ctz(e);
                        size_type slot_ix = (ix + skip) & (N - 1);

                        // invariant: slot_ix in [0 .. N)

                        auto & slot = slots_[slot_ix];

                        // mark slot occupied in control space;
                        // maintain copy-at-end for overflow
                        this->_update_control(slot_ix, h2);
                        new (&slot) value_type(kv_pair);

                        ++(this->size_);

                        // true: increased table size
                        return std::make_pair(&slot, true);
                    }
                }

                // slot range associated with grp
                // has no room, and does not contain target key
                // -> move on to next group.
                //
                // note: relying on c_group_size overflow bytes here
                // when ix is close to N

                ix = (ix + c_group_size) & (N - 1);
            }
        }

        /**
         *  Verify DArenaHashMap class invariants.
         *
         *  SM1. size consistency
         *   - SM1.1 size_ <= n_slot_
         *   - SM1.2 control_[] size consistent with slots_[] size
         *   - SM1.3 n_group_ consistent with n_group_exponent_
         *   - SM1.4 n_slot_ consistent with n_group_
         *   - SM1.5 n_slot_ a power of 2
         *  SM2. load factor
         *   - SM2.1 load_factor() <= c_max_load_factor
         *  SM3. control_
         *   - SM3.1 control_[N+i] = control_[i] for i in [0, c_group_size)
         *   - SM3.2 {number of control_[i] spots with non-sentinel values} = size_
         *  SM4. slots_
         *   - SM4.1 if control_[i] is non-sentinel:
         *     - SM4.1.1 control_[i] = hash_(slots_[i].first) & 0x7f
         *     - SM4.1.2 all slots in range [h .. i] are non-empty,
         *       where h is hash_(slots_[i].first >> 7
         *   - SM4.2 if control_[i] is empty or tombstone:
         *     - slots_[i].first = key_type()
         *
         **/
        template <typename Key, typename Value, typename Hash, typename Equal>
        bool
        DArenaHashMap<Key, Value, Hash, Equal>::verify_ok(verify_policy policy) const
        {
            using xo::scope;
            using xo::tostr;
            using xo::xtag;

            constexpr const char * c_self = "DArenaHashMap::verify_ok";
            scope log(XO_DEBUG(debug_flag_), xtag("size", size_));

            /* SM1.1: size_ <= n_slot_ */
            if (size_ > n_slot_) {
                return policy.report_error(log,
                                           c_self, ": expect .size <= .n_slot",
                                           xtag("size", size_),
                                           xtag("n_slot", n_slot_));
            }

            /* SM1.2: control_[] size consistent with slots_[] size */
            if (control_.size() != n_slot_ + c_group_size) {
                return policy.report_error(log,
                                           c_self, ": expect .control_.size = .n_slot + c_group_size",
                                           xtag("control_.size", control_.size()),
                                           xtag("n_slot", n_slot_),
                                           xtag("c_group_size", c_group_size));
            }
            if (slots_.size() != n_slot_) {
                return policy.report_error(log,
                                           c_self, ": expect .slots_.size = .n_slot",
                                           xtag("slots_.size", slots_.size()),
                                           xtag("n_slot", n_slot_));
            }

            /* SM1.3: n_group_ consistent with n_group_exponent_ */
            if (n_group_ != (size_type{1} << n_group_exponent_)) {
                return policy.report_error(log,
                                           c_self, ": expect .n_group = 2^.n_group_exponent",
                                           xtag("n_group", n_group_),
                                           xtag("n_group_exponent", n_group_exponent_));
            }

            /* SM1.4: n_slot_ consistent with n_group_ */
            if (n_slot_ != n_group_ * c_group_size) {
                return policy.report_error(log,
                                           c_self, ": expect .n_slot = .n_group * c_group_size",
                                           xtag("n_slot", n_slot_),
                                           xtag("n_group", n_group_),
                                           xtag("c_group_size", c_group_size));
            }

            /* SM1.5: n_slot_ a power of 2 */
            if ((n_slot_ & (n_slot_ - 1)) != 0) {
                return policy.report_error(log,
                                           c_self, ": expect .n_slot is power of 2",
                                           xtag("n_slot", n_slot_));
            }

            /* SM2.1: load_factor() <= c_max_load_factor */
            if (load_factor() > c_max_load_factor) {
                return policy.report_error(log,
                                           c_self, ": expect .load_factor <= c_max_load_factor",
                                           xtag("load_factor", load_factor()),
                                           xtag("c_max_load_factor", c_max_load_factor));
            }

            /* SM3.1: control_[N+i] = control_[i] for i in [0, c_group_size) */
            for (size_type i = 0; i < c_group_size; ++i) {
                if (control_[n_slot_ + i] != control_[i]) {
                    return policy.report_error(log,
                                               c_self, ": expect control_[N+i] = control_[i]",
                                               xtag("i", i),
                                               xtag("control_[i]", control_[i]),
                                               xtag("control_[N+i]", control_[n_slot_ + i]));
                }
            }

            /* SM3.2: {number of control_[i] spots with non-sentinel values} = size_ */
            {
                size_type occupied_count = 0;
                for (size_type i = 0; i < n_slot_; ++i) {
                    uint8_t c = control_[i];
                    if ((c != c_empty_slot) && (c != c_tombstone)) {
                        ++occupied_count;
                    }
                }
                if (occupied_count != size_) {
                    return policy.report_error(log,
                                               c_self, ": expect occupied control count = size",
                                               xtag("occupied_count", occupied_count),
                                               xtag("size", size_));
                }
            }

            /* SM4.1.1: if control_[i] is non-sentinel, control_[i] = hash_(slots_[i].first) & 0x7f */
            for (size_type i = 0; i < n_slot_; ++i) {
                uint8_t c = control_[i];
                if ((c != c_empty_slot) && (c != c_tombstone)) {
                    uint8_t expected_h2 = hash_(slots_[i].first) & 0x7f;
                    if (c != expected_h2) {
                        return policy.report_error(log,
                                                   c_self, ": expect control[i] = hash(key) & 0x7f",
                                                   xtag("i", i),
                                                   xtag("control[i]", c),
                                                   xtag("expected_h2", expected_h2));
                    }
                }
            }

            /* SM4.1.2: if control_[i] is non-sentinel, all slots in range [h .. i] are non-empty,
             *          where h = (hash_(slots_[i].first) >> 7) & (n_slot_ - 1)
             */
            for (size_type i = 0; i < n_slot_; ++i) {
                uint8_t c = control_[i];
                if ((c != c_empty_slot) && (c != c_tombstone)) {
                    size_type h = (hash_(slots_[i].first) >> 7) & (n_slot_ - 1);
                    size_type j = h;
                    while (j != i) {
                        uint8_t cj = control_[j];
                        if ((cj == c_empty_slot) || (cj == c_tombstone)) {
                            return policy.report_error(log,
                                                       c_self, ": expect non-empty slot in probe range [h..i]",
                                                       xtag("i", i),
                                                       xtag("h", h),
                                                       xtag("j", j),
                                                       xtag("control[j]", cj));
                        }
                        j = (j + 1) & (n_slot_ - 1);
                    }
                }
            }

            return true;
        }
    }
} /*namespace xo*/

/* end DArenaHashMap.hpp */
