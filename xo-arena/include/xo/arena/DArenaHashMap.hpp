/** @file DArenaHashMap.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "DArenaVector.hpp"
#include <utility>

namespace xo {
    namespace mm {
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
        struct DArenaHashMap {
        public:
            using size_type = std::size_t;
            using key_type = Key;
            using mapped_type = Value;
            using value_type = std::pair<const Key, Value>;
            using key_hash = Hash;
            using key_equal = Equal;
            using byte = std::byte;

            /** create hash map **/
            DArenaHashMap(size_type hint_max_capacity,
                          bool debug_flag = false);
            DArenaHashMap(Hash && hash = Hash(),
                          Equal && eq = Equal(),
                          size_type hint_max_capacity = 0,
                          bool debug_flag = false);

            /** find smallest x such that 2^x >= n. Return {x, 2^x} **/
            static std::pair<size_type, size_type> lub_exp2(size_t n);
            static constexpr size_type group_size() { return c_group_size; }
#ifdef NOT_YET
            static size_type min_groups();
            static size_type min_size() { return min_groups() * c_group_size; }
#endif

            size_type empty() const noexcept { return size_ == 0; }
            size_type capacity() const noexcept { return n_group_ * c_group_size; }

#ifdef NOT_YET
            // TODO: std::pair<iterator, bool>
            void
            insert(std::pair<const Key, Value> & kv_pair) {
                uint64_t h = hash_(kv_pair.first);
            }
#endif

        private:
            /** group size **/
            static constexpr std::size_t c_group_size = 16;

            /** hash function **/
            key_hash hash_;
            /** key equal **/
            key_equal equal_;
            /** number of pairs in this table **/
            std::size_t size_ = 0;
            /** base-2 logarithm of n_group_ **/
            std::size_t n_group_exponent_ = 0;
            /** table has capacity for this number of groups. always an exact power of two.
             *  number of slots is n_group_ * c_group_size
             **/
            std::size_t n_group_ = 1 << n_group_exponent_;
            /** control_[] partitioned into groups of c_group_size (16) consecutive elements **/
            DArenaVector<byte> control_;
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

        template <typename Key, typename Value, typename Hash, typename Equal>
        DArenaHashMap<Key, Value, Hash, Equal>::DArenaHashMap(Hash && hash,
                                                              Equal && eq,
                                                              size_type hint_max_capacity,
                                                              bool debug_flag)
        : hash_{std::move(hash)},
          equal_{std::move(eq)},
          size_{0},
          n_group_exponent_{lub_exp2(hint_max_capacity).first},
          n_group_{lub_exp2(hint_max_capacity).second},
          control_{DArenaVector<byte>::map(ArenaConfig{.size_ = n_group_})},
          slots_{DArenaVector<value_type>::map(ArenaConfig{.size_ = n_group_ * sizeof(value_type)})},
          debug_flag_{debug_flag}
        {
        }

        template <typename Key, typename Value, typename Hash, typename Equal>
        auto
        DArenaHashMap<Key, Value, Hash, Equal>::lub_exp2(size_t n) -> std::pair<size_type, size_type>

        {
            size_type ngx = 0;
            size_type ng = 1;

            while (ng < n) {
                ++ngx;
                ng *= 2;
            }

            return std::make_pair(ngx, ng);;
        }

#ifdef NOT_YET
        template <typename Key, typename Value, typename Hash, typename Equal>
        auto
        DArenaHashMap<Key, Value, Hash, Equal>::min_groups() -> size_type
        {
            size_type page_z = getpagesize();

            // 1 page of slots
            size_type n_slot = page_z / sizeof(value_type);

            // 1 page of groups
            size_type n_group = n_slot / c_group_size;

            // glb power of 2, but at least 1
            size_type ng = 1;

            while (2 * ng < n_group)
                ng *= 2;

            return ng;
        }
#endif

    }
} /*namespace xo*/

/* end DArenaHashMap.hpp */
