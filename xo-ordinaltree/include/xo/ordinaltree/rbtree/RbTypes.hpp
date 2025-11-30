/** @file RbTypes.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "NullReduce.hpp"

namespace xo {
    namespace tree {
        /* red-black tree with order statistics
         *
         * require:
         * - Key is equality comparable
         * - Key, Value, Reduce are copyable and null-constructible
         * - Reduce.value_type = Accumulator
         * - Reduce.operator() :: (Accumulator x Key) -> Accumulator
         * - Reduce.operator() :: (Accumulator x Accumulator) -> Accumulator
         */
        template <typename Key,
                  typename Value,
                  typename Reduce = NullReduce<Key>,
                  typename Allocator = std::allocator<std::pair<const Key, Value>>>
        class RedBlackTree;

        namespace detail {
            enum Color { C_Invalid = -1, C_Black, C_Red, N_Color };

            enum Direction { D_Invalid = -1, D_Left, D_Right, N_Direction };

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
    } /*namespace tree*/
} /*namespace xo*/

/* end RbTypes.hpp */
