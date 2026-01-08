/** @file DArenaHashMap.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "DArenaVector.hpp"
#include "hashmap/DArenaHashMapUtil.hpp"
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

            template <typename Key,
                      typename Value>
            struct HashMapStore : DArenaHashMapUtil {
            public:
                using value_type = std::pair<const Key, Value>;
                using group_type = detail::Group;

            public:
                /** group_exp2: number of groups {x, 2^x} **/
                explicit HashMapStore(const std::pair<size_type,
                                                      size_type> & group_exp2)
                : size_{0},
                  n_group_exponent_{group_exp2.first},
                  n_group_{group_exp2.second},
                  n_slot_{group_exp2.second * c_group_size},
                  control_{DArenaVector<uint8_t>::map(ArenaConfig{.size_ = control_size(n_slot_)})},
                  slots_{DArenaVector<value_type>::map(ArenaConfig{.size_ = n_slot_ * sizeof(value_type)})}
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
                DArenaVector<uint8_t> control_;
                /** slots_[] holds {key,value} pairs **/
                DArenaVector<value_type> slots_;
            };

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
            using store_type = detail::HashMapStore<Key, Value>;
            using insert_value_type = std::pair<value_type *, bool>;
            using iterator = detail::DArenaHashMapIterator<Key, Value>;

            /** create hash map **/
            DArenaHashMap(size_type hint_max_capacity,
                          bool debug_flag = false);
            DArenaHashMap(Hash && hash = Hash(),
                          Equal && eq = Equal(),
                          size_type hint_max_capacity = 0,
                          bool debug_flag = false);

            size_type empty() const noexcept { return store_.empty(); }
            size_type groups() const noexcept { return store_.n_group_; }
            size_type size() const noexcept { return store_.size_; }
            size_type capacity() const noexcept { return store_.capacity(); }
            float load_factor() const noexcept { return store_.load_factor(); }

            bool verify_ok(verify_policy p = verify_policy::throw_only()) const;

            iterator begin() {
                if (this->empty()) [[unlikely]] {
                    return this->end();
                }

                iterator ix(&(store_.control_[c_control_stub]),
                            &(store_.slots_[0]));

                if (ix._at_slot_sentinel()) {
                    /* advance to first occupied position in table */
                    ++ix;
                }

                return ix;
            }

            iterator end() {
                iterator ix(&(store_.control_[c_control_stub + store_.capacity()]),
                            &(store_.slots_[store_.capacity()]));

                return ix;
            }

            /** insert @p kv_pair into hash map.
             *  Replaces any previous value stored under the same key.
             *
             *  Return pair retval with:
             *  reval.first: true if size incremented;
             *  retval.second: address of slots_[p] at which pair inserted/updated
             *
             *  When table is full retval.second will be nullptr,
             *  with error captured in last_error_
             **/
            insert_value_type try_insert(const value_type & kv_pair);

            /** insert @p kv_pair into hash map.
             *  Increase table size if necessary
             **/
            bool insert(const value_type & kv_pair);

            /** reset to empty state **/
            void clear();

            /** find element with key @p key.
             *  @return iterator to element if found, end() otherwise
             **/
            iterator find(const key_type & key);

        private:
            /** insert @p kv_pair,
             *  where key hashes to @p hash_value, into @p *store
             **/
            insert_value_type _try_insert_aux(size_type hash_value,
                                              const value_type & kv_pair,
                                              store_type * p_store);

            /** increase hash table size (invoke when max load factor reached) **/
            bool _try_grow();

            /** load group abstraction from control bytes starting at @p ix **/
            group_type _load_group(size_type ix) { return store_._load_group(ix); }

            /** like ctrl_[ix] = h2, but maintain overflow copy
             *  at end of ctrl_[] array
             **/
            void _update_control(size_type ix, uint8_t h2) {
                return store_._update_control(ix, h2);
            }

        private:
            /** hash function **/
            key_hash hash_;
            /** key equal **/
            key_equal equal_;
            /** hash table state contents + size-related attributes **/
            store_type store_;
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
          store_{lub_exp2(lub_group_mult(hint_max_capacity))},
          debug_flag_{debug_flag}
        {
        }

        template <typename Key, typename Value, typename Hash, typename Equal>
        auto
        DArenaHashMap<Key,
                      Value,
                      Hash,
                      Equal>::try_insert(const value_type & kv_pair) -> insert_value_type
        {
            size_type h = hash_(kv_pair.first);

            return _try_insert_aux(h, kv_pair, &store_);
        }

        template <typename Key,
                  typename Value,
                  typename Hash,
                  typename Equal>
        auto
        DArenaHashMap<Key,
                      Value,
                      Hash,
                      Equal>::_try_insert_aux(size_type hash_value,
                                              const std::pair<const Key, Value> & kv_pair,
                                              store_type * p_store)
            -> std::pair<value_type *, bool>

        {
            scope log(XO_DEBUG(false));

            size_type h = hash_value;
            // h1: hi bits: probe sequence
            size_type h1 = h >> 7;
            // h2: lo bits: store in control byte
            uint8_t h2 = h & 0x7f;

            size_type N = p_store->capacity();

            if (N == 0) [[unlikely]] {
                return std::make_pair(nullptr, false);
            }

            // same as:
            //   ix = h1 % N
            // since N is power of 2
            size_type ix = h1 & (N - 1);

            // will make series of probes
            for (;;) {
                auto grp = p_store->_load_group(ix);

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

                        auto & slot = p_store->slots_[slot_ix];

                        if (equal_(slot.first, kv_pair.first)) {
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
                        if (p_store->load_factor() >= c_max_load_factor) {
                            return std::make_pair(nullptr, false);
                        }

                        // zeroes: #of 0 before least significant 1 bit
                        int skip = __builtin_ctz(e);
                        size_type slot_ix = (ix + skip) & (N - 1);

                        // invariant: slot_ix in [0 .. N)

                        auto & slot = p_store->slots_[slot_ix];

                        // mark slot occupied in control space;
                        // maintain copy-at-end for overflow
                        p_store->_update_control(slot_ix, h2);
                        new (&slot) value_type(kv_pair);

                        ++(p_store->size_);

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

        template <typename Key, typename Value, typename Hash, typename Equal>
        bool
        DArenaHashMap<Key, Value, Hash, Equal>::_try_grow()
        {
            scope log(XO_DEBUG(false));

            size_type n_group_exponent_2x = 0;
            size_type n_group_2x = 0;

            if (store_.n_group_ == 0) [[unlikely]] {
                // special case: grow from hard empty state
                n_group_exponent_2x = 0;
                n_group_2x = 1;
            } else {
                n_group_exponent_2x = store_.n_group_exponent_ + 1;
                n_group_2x = 2 * n_group_exponent_2x;
            }

            // optimization when table is empty.  in that case can resize
            // arenas in place

            if (this->empty()) {
                log && log("resize-from-empty branch");

                this->store_.resize_from_empty(std::make_pair(n_group_exponent_2x, n_group_2x));
            } else {
                log && log("duplicate-and-replace branch");

                detail::HashMapStore<Key, Value> store_2x(std::make_pair(n_group_exponent_2x,
                                                                         n_group_2x));
                /* rehash everything in store_,
                 * into store_2x
                 */

                for (size_type i = 0, n = store_.capacity(); i < n; ++i) {
                    uint8_t ctrl = store_.control_[c_control_stub + i];
                    value_type & kv_pair = store_.slots_[i];

                    if (DArenaHashMapUtil::is_data(ctrl)) {
                        size_type h = hash_(kv_pair.first);
                        auto chk = this->_try_insert_aux(h, kv_pair, &store_2x);

                        if (!chk.second) {
                            // shenanigans - something isn't right.
                            // - may have run out of memory
                            assert(false);

                            return false;
                        }
                    }
                }

                this->store_ = std::move(store_2x);
            }

            return true;
        }

        template <typename Key,
                  typename Value,
                  typename Hash,
                  typename Equal>
        bool
        DArenaHashMap<Key,
                      Value,
                      Hash,
                      Equal>::insert(const std::pair<const Key, Value> & kv_pair)
        {
            scope log(XO_DEBUG(false));

            auto [slot_addr, ins_flag] = this->try_insert(kv_pair);

            if (slot_addr) {
                log && log("fast", xtag("slot_addr", (void*)slot_addr), xtag("ins_flag", ins_flag));

                return ins_flag;
            }

            assert((store_.size_ + 1) / static_cast<float>(store_.n_slot_) >= c_max_load_factor);

            if (this->_try_grow()) {
                /* retry insert, with bigger table */
                auto [slot_addr, ins_flag] = this->try_insert(kv_pair);

                return ins_flag;
            } else {
                assert(false);

                // TODO: set last error.  Presumeably reached max size
                return false;
            }
        }

        template <typename Key,
                  typename Value,
                  typename Hash,
                  typename Equal>
        void
        DArenaHashMap<Key, Value, Hash, Equal>::clear()
        {
            this->store_.clear();
        }

        template <typename Key,
                  typename Value,
                  typename Hash,
                  typename Equal>
        auto
        DArenaHashMap<Key, Value, Hash, Equal>::find(const key_type & key) -> iterator
        {
            size_type N = store_.capacity();

            if (N == 0) [[unlikely]] {
                return this->end();
            }

            size_type h = hash_(key);
            size_type h1 = h >> 7;
            uint8_t h2 = h & 0x7f;

            size_type ix = h1 & (N - 1);

            for (;;) {
                auto grp = store_._load_group(ix);

                {
                    uint16_t m = grp.all_matches(h2);

                    while (m) {
                        int skip = __builtin_ctz(m);
                        size_type slot_ix = (ix + skip) & (N - 1);

                        auto & slot = store_.slots_[slot_ix];

                        if (equal_(slot.first, key)) {
                            return iterator(&(store_.control_[c_control_stub + slot_ix]),
                                            &slot);
                        }

                        m &= (m - 1);
                    }
                }

                {
                    uint16_t e = grp.empty_matches();

                    if (e) {
                        return this->end();
                    }
                }

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
         *   - SM3.1 control_[i] = c_iterator_bookend for i in [0, c_control_stub)
         *   - SM3.2 control_[stub+i] = control_[stub+N+i] for i in [0, c_group_size)
         *   - SM3.3 {number of control_[i] spots with non-sentinel values} = size_
         *   - SM3.4 control_[stub+N+c_group_size+i] = c_iterator_bookend for i in [0, c_control_stub)
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
            scope log(XO_DEBUG(debug_flag_),
                      xtag("size", store_.size_));

            /* SM1.1: size_ <= n_slot_ */
            if (store_.size_ > store_.n_slot_) {
                return policy.report_error(log,
                                           c_self, ": expect .size <= .n_slot",
                                           xtag("size", store_.size_),
                                           xtag("n_slot", store_.n_slot_));
            }

            /* SM1.2: control_[] size consistent with slots_[] size */
            if (store_.control_.size() != control_size(store_.n_slot_)) {
                return policy.report_error
                           (log,
                            c_self, ": expect .control_.size = .n_slot + c_group_size + 2 * c_control_stub",
                            xtag("control_.size", store_.control_.size()),
                            xtag("n_slot", store_.n_slot_),
                            xtag("c_group_size", c_group_size),
                            xtag("c_control_stub", c_control_stub));
            }
            if (store_.slots_.size() != store_.n_slot_) {
                return policy.report_error(log,
                                           c_self, ": expect .slots_.size = .n_slot",
                                           xtag("slots_.size", store_.slots_.size()),
                                           xtag("n_slot", store_.n_slot_));
            }

            /* SM1.3: n_group_ consistent with n_group_exponent_ */
            if (store_.n_group_ != (size_type{1} << store_.n_group_exponent_)) {
                return policy.report_error(log,
                                           c_self, ": expect .n_group = 2^.n_group_exponent",
                                           xtag("n_group", store_.n_group_),
                                           xtag("n_group_exponent", store_.n_group_exponent_));
            }

            /* SM1.4: n_slot_ consistent with n_group_ */
            if (store_.n_slot_ != store_.n_group_ * c_group_size) {
                return policy.report_error(log,
                                           c_self, ": expect .n_slot = .n_group * c_group_size",
                                           xtag("n_slot", store_.n_slot_),
                                           xtag("n_group", store_.n_group_),
                                           xtag("c_group_size", c_group_size));
            }

            /* SM1.5: n_slot_ a power of 2 */
            if ((store_.n_slot_ & (store_.n_slot_ - 1)) != 0) {
                return policy.report_error(log,
                                           c_self, ": expect .n_slot is power of 2",
                                           xtag("n_slot", store_.n_slot_));
            }

            /* SM2.1: load_factor() <= c_max_load_factor */
            if (load_factor() > c_max_load_factor) {
                return policy.report_error(log,
                                           c_self, ": expect .load_factor <= c_max_load_factor",
                                           xtag("load_factor", load_factor()),
                                           xtag("c_max_load_factor", c_max_load_factor));
            }

            /* SM3.1: control_[i] = c_iterator_bookend for i in [0, c_control_stub) */
            for (size_type i = 0; i < c_control_stub; ++i) {
                if (store_.control_[i] != c_iterator_bookend) {
                    return policy.report_error(log,
                                               c_self, ": expect control_[i] = c_iterator_bookend for front stub",
                                               xtag("i", i),
                                               xtag("control_[i]", (int)(store_.control_[i])),
                                               xtag("c_iterator_bookend", (int)c_iterator_bookend));
                }
            }

            /* SM3.2: control_[N+i] = control_[i] for i in [0, c_group_size) */
            for (size_type i = 0; i < c_group_size; ++i) {
                if (store_.control_[store_.n_slot_ + i + c_control_stub] != store_.control_[i + c_control_stub]) {
                    return policy.report_error(log,
                                               c_self, ": expect control_[N+i] = control_[i]",
                                               xtag("i", i),
                                               xtag("control_[i]", (int)(store_.control_[i + c_control_stub])),
                                               xtag("control_[N+i]", (int)(store_.control_[store_.n_slot_ + i + c_control_stub])));
                }
            }

            /* SM3.3: {number of control_[i] spots with non-sentinel values} = size_ */
            {
                size_type occupied_count = 0;
                for (size_type i = 0; i < store_.n_slot_; ++i) {
                    uint8_t c = store_.control_[i + c_control_stub];
                    if (DArenaHashMapUtil::is_data(c)) {
                        ++occupied_count;
                    }
                }
                if (occupied_count != store_.size_) {
                    return policy.report_error(log,
                                               c_self, ": expect occupied control count = size",
                                               xtag("occupied_count", occupied_count),
                                               xtag("size", store_.size_));
                }
            }

            /* SM3.4: control_[stub+N+c_group_size+i] = c_iterator_bookend for i in [0, c_control_stub) */
            for (size_type i = 0; i < c_control_stub; ++i) {
                size_type ix = c_control_stub + store_.n_slot_ + c_group_size + i;
                if (store_.control_[ix] != c_iterator_bookend) {
                    return policy.report_error(log,
                                               c_self, ": expect control_[stub+N+group+i] = c_iterator_bookend for end stub",
                                               xtag("i", i),
                                               xtag("ix", ix),
                                               xtag("control_[ix]", (int)(store_.control_[ix])),
                                               xtag("c_iterator_bookend", (int)c_iterator_bookend));
                }
            }

            /* SM4.1.1: if control_[i] is non-sentinel, control_[i] = hash_(slots_[i].first) & 0x7f */
            for (size_type i = 0; i < store_.n_slot_; ++i) {
                uint8_t c = store_.control_[i + c_control_stub];
                if (DArenaHashMapUtil::is_data(c)) {
                    uint8_t expected_h2 = hash_(store_.slots_[i].first) & 0x7f;
                    if (c != expected_h2) {
                        return policy.report_error(log,
                                                   c_self, ": expect control[i] = hash(key) & 0x7f",
                                                   xtag("i", i),
                                                   xtag("control[i+stub]", c),
                                                   xtag("expected_h2", expected_h2));
                    }
                }
            }

            /* SM4.1.2: if control_[i] is non-sentinel, all slots in range [h .. i] are non-empty,
             *          where h = (hash_(slots_[i].first) >> 7) & (n_slot_ - 1)
             */
            for (size_type i = 0; i < store_.n_slot_; ++i) {
                uint8_t c = store_.control_[i + c_control_stub];
                if (DArenaHashMapUtil::is_data(c)) {
                    size_type h = (hash_(store_.slots_[i].first) >> 7) & (store_.n_slot_ - 1);
                    size_type j = h;
                    while (j != i) {
                        uint8_t cj = store_.control_[j + c_control_stub];
                        if (DArenaHashMapUtil::is_sentinel(cj)) {
                            return policy.report_error(log,
                                                       c_self, ": expect non-empty slot in probe range [h..i]",
                                                       xtag("i", i),
                                                       xtag("h", h),
                                                       xtag("j", j),
                                                       xtag("control[j+stub]", cj));
                        }
                        j = (j + 1) & (store_.n_slot_ - 1);
                    }
                }
            }

            /* SM4.2: if control_[i] is empty or tombstone, slots_[i].first = key_type() */
            for (size_type i = 0; i < store_.n_slot_; ++i) {
                uint8_t c = store_.control_[i + c_control_stub];
                if (DArenaHashMapUtil::is_sentinel(c)) {
                    if (!(store_.slots_[i].first == key_type())) {
                        return policy.report_error(log,
                                                   c_self, ": expect empty/tombstone slot has default key",
                                                   xtag("i", i),
                                                   xtag("control[i+stub]", c));
                    }
                }
            }

            return true;
        }
    }
} /*namespace xo*/

/* end DArenaHashMap.hpp */
