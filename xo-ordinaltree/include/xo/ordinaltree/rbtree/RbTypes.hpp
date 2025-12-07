/** @file RbTypes.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "NullReduce.hpp"
#include <compare>

namespace xo {
    namespace tree {
        /**
         * Wrapper to workaround concept requirements that prevent
         * std::compare_three_way working with const references
         **/
        struct DefaultThreeWayCompare {
#ifdef NOT_YET // not working with clang 16.0.6
            template <typename T>
            auto operator()(const T & a, const T & b) const {
                return a <=> b;
            }
#endif

            template <typename T>
            auto operator()(const T& a, const T& b) const -> std::strong_ordering {
                if constexpr (std::three_way_comparable<T>) {
                    return a <=> b;
                } else {
                    if (a < b) return std::strong_ordering::less;
                    if (b < a) return std::strong_ordering::greater;
                    return std::strong_ordering::equal;
                }
            }
        };

        /**
         * Red-black tree with order statistics
         *
         * Require:
         * - Key is equality comparable
         * - Key, Value, Reduce are copyable and null-constructible
         * - Reduce.value_type = Accumulator
         * - Reduce.operator() :: (Accumulator x Key) -> Accumulator
         * - Reduce.operator() :: (Accumulator x Accumulator) -> Accumulator
         **/
        template <typename Key,
                  typename Value,
                  typename Reduce = NullReduce<Key>,
                  typename Compare = DefaultThreeWayCompare, //std::compare_three_way, //std::less<Key>,
                  typename Allocator = std::allocator<std::pair<const Key, Value>>>
        class RedBlackTree;

        namespace detail {
            enum Color {
                C_Invalid = -1,
                C_Black,
                C_Red,
                N_Color };

            inline const char *
            color2str(Color x) {
                switch(x) {
                case C_Black: return "B";
                case C_Red: return "r";
                default: return "?color";
                }
            }

            enum Direction {
                D_Invalid = -1,
                D_Left,
                D_Right,
                N_Direction
            };

            inline Direction other(Direction d) {
                return static_cast<Direction>(1 - d);
            } /*other*/

            enum IteratorDirection {
                /* ID_Forward.    forward iterator
                 * ID_Reverse.    reverse iterator
                 */
                ID_Forward,
                ID_Reverse
            }; /*IteratorDirection*/

            /* specify iterator location relative to Iterator::node.
             * using this to make it possible to correctly decrement an
             * iterator at RedBlackTree::end().
             *
             * IL_BeforeBegin.    if non-empty tree, .node is the first node
             *                    in the tree (the one with smallest key),
             *                    and iterator refers to the location
             *                    "one before" that first node.
             * IL_Regular.        iterator refers to member of the tree
             *                    given by Iterator::node
             * IL_AfterEnd.       if non-empty tree, .node is the last node
             *                    in the tree (the one with largest key),
             *                    and iterator refers the the location
             *                    "one after" that last node.
             */
            enum IteratorLocation {
                IL_BeforeBegin,
                IL_Regular,
                IL_AfterEnd,
            }; /*IteratorLocation*/
        } /*namespace detail*/

        template <typename Key>
        concept ordered_key = (std::copyable<Key>
                               && std::default_initializable<Key>
                               && std::totally_ordered<Key>);


        template <typename Compare,
                  typename Key>
        concept three_way_comparator = requires(const Compare& comp,
                                                const Key& a,
                                                const Key& b)
        {
            { comp(a, b) } -> std::same_as<std::strong_ordering>;
        };

        template <typename Value>
        concept valid_rbtree_node_value = (std::copyable<Value>
                                           && std::default_initializable<Value>);

        /* concept for the 'Reduce' argument to RedBlackTree<...>
         *
         * here:
         *   T             = class implementing reduce feature,  e.g. SumReduce<...>
         *   T::value_type = type for output of reduce function.
         *
         *   Value         = value_type for rb-tree that supports ordinal statistics
         *
         * e.g.
         *   struct ReduceCountAndSum {
         *     using value_type = std::pair<uint32_t, int64_t>:
         *
         *     value_type nil() { return value_type(0, 0); }
         *     value_type operator()(value_type const & acc, int64_t val)
         *       { return value_type(acc.first + val.first, acc.second + val.second); }
         *     value_type operator()(value_type const & a1, value_type const & a2)
         *       { return value_type(a1.first + a2.first, a1.second + a2.second); }
         *   };
         *
         *   Reduce.nil() -> nominal reduction i.e. reduce on empty set
         *   Reduce.leaf(v) -> reduction on set {v}
         *
         * in general: at some internal node, tree splits set of key/value pairs on some key k1,
         * with a left subtree lh,  and a right subtree rh.
         *
         * for a binary tree we want to maintain:
         * - r1: reduce applied to collection
         *    lh + {k1} = reduce(reduce(lh), k1)
         * - r2: reduce applied to collection
         *    lh + {k1} + rh = reduce.combine(r1, reduce(r2))
         *
         */
        template <typename Reduce, typename Value>
        concept valid_rbtree_reduce_functor = requires(const Reduce & reduce,
                                                       const Value & value,
                                                       typename Reduce::value_type const & rv1,
                                                       typename Reduce::value_type const & rv2)
        {
            typename Reduce::value_type;

            { reduce.nil() } -> std::convertible_to<typename Reduce::value_type>;
            { reduce.leaf(value) } -> std::convertible_to<typename Reduce::value_type>;
            { reduce.combine(rv1, rv2) } -> std::convertible_to<typename Reduce::value_type>;

            requires std::default_initializable<Reduce>;
        };

        template <typename Key, typename Value, typename Reduce, typename GcObjectInterface>
        concept valid_rbtree_node_params = (ordered_key<Key>
                                            && valid_rbtree_node_value<Value>
                                            && valid_rbtree_reduce_functor<Reduce, Value>
                                            );

    } /*namespace tree*/
} /*namespace xo*/

/* end RbTypes.hpp */
