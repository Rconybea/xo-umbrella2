/** @file MapTdx.test.cpp
 *
 *  reflection for std::map, std::unordered_map -- see
 *  .xo-backlog/xo-reflect/issues/05.
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "xo/reflect/Reflect.hpp"
#include <catch2/catch.hpp>
#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace xo {
    using xo::reflect::Metatype;
    using xo::reflect::Ownership;
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TypeDescr;

    namespace ut {
        namespace {
            /** a key with no operator< and a hash: an unordered map of these
             *  cannot be sorted
             **/
            struct Unordered {
                int v_;
                bool operator==(Unordered const & x) const { return v_ == x.v_; }
            };

            struct UnorderedHash {
                std::size_t operator()(Unordered const & x) const { return std::hash<int>()(x.v_); }
            };

            /** keys and value addresses, as visit_entries() gives them **/
            template <typename K, typename V>
            std::vector<std::pair<K, void *>> entries_of(TaggedPtr tp) {
                std::vector<std::pair<K, void *>> retval;

                tp.td()->map_info()->visit_entries(tp.address(),
                                                   [&retval](TaggedPtr k, TaggedPtr v) {
                                                       retval.emplace_back(*k.recover_native<K>(), v.address());
                                                   });
                return retval;
            }
        } /*namespace*/

        TEST_CASE("std-map-reflects-as-a-map", "[reflect][map]") {
            std::map<std::string, int> m{{"b", 2}, {"a", 1}, {"c", 3}};

            TaggedPtr tp = Reflect::make_tp(&m);
            TypeDescr td = tp.td();

            REQUIRE(td->metatype() == Metatype::mt_map);
            REQUIRE(td->is_map());
            REQUIRE(td->map_info() != nullptr);
            REQUIRE(td->map_info()->key_td() == Reflect::require<std::string>());
            REQUIRE(td->map_info()->value_td() == Reflect::require<int>());
            REQUIRE(td->fixed_child_td(0) == Reflect::require<int>());
            REQUIRE(tp.n_child() == 3);
            /* entries held inline */
            REQUIRE(td->child_edge_ownership() == Ownership::owning);

            /* key order, each value in place */
            auto e = entries_of<std::string, int>(tp);
            REQUIRE(e.size() == 3);
            REQUIRE(e[0].first == "a");
            REQUIRE(e[0].second == &m["a"]);
            REQUIRE(e[1].first == "b");
            REQUIRE(e[2].first == "c");
            REQUIRE(e[2].second == &m["c"]);

            /* the i'th value, in key order */
            REQUIRE(tp.get_child(1).address() == &m["b"]);
        }

        TEST_CASE("std-unordered-map-in-key-order", "[reflect][map]") {
            std::unordered_map<std::uint64_t, std::string> m;
            for (std::uint64_t k : {42, 7, 1000, 3, 99, 1})
                m[k] = "v" + std::to_string(k);

            TaggedPtr tp = Reflect::make_tp(&m);

            REQUIRE(tp.td()->metatype() == Metatype::mt_map);

            /* sorted: deterministic whatever the hashing */
            auto e = entries_of<std::uint64_t, std::string>(tp);
            std::vector<std::uint64_t> keys;
            for (auto const & x : e)
                keys.push_back(x.first);

            REQUIRE(keys == std::vector<std::uint64_t>{1, 3, 7, 42, 99, 1000});
            REQUIRE(e[3].second == &m[42]);
        }

        TEST_CASE("unordered-map-unsortable-keys", "[reflect][map]") {
            /* no operator<: every entry, in iteration order */
            std::unordered_map<Unordered, int, UnorderedHash> m{{{1}, 10}, {{2}, 20}, {{3}, 30}};

            TaggedPtr tp = Reflect::make_tp(&m);

            int n = 0;
            int sum = 0;
            tp.td()->map_info()->visit_entries(tp.address(),
                                               [&](TaggedPtr, TaggedPtr v) {
                                                   ++n;
                                                   sum += *v.recover_native<int>();
                                               });

            REQUIRE(n == 3);
            REQUIRE(sum == 60);
        }

        TEST_CASE("std-map-empty", "[reflect][map]") {
            std::map<int, int> m;

            TaggedPtr tp = Reflect::make_tp(&m);

            REQUIRE(tp.td()->is_map());
            REQUIRE(tp.n_child() == 0);
            REQUIRE(entries_of<int, int>(tp).empty());
        }

        TEST_CASE("map-not-for-other-types", "[reflect][map]") {
            REQUIRE(!Reflect::require<std::vector<int>>()->is_map());
            REQUIRE(Reflect::require<std::vector<int>>()->map_info() == nullptr);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end MapTdx.test.cpp */
