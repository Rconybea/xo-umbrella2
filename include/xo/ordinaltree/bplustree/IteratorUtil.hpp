/* @file IteratorUtil.hpp */

#pragma once

#include <ostream>

namespace xo {
    namespace tree {
        namespace detail {

            enum IteratorDirection {
                /* ID_Forward.    forward iterator
                 * ID_Reverse.    reverse iterator
                 */
                ID_Forward,
                ID_Reverse
            }; /*IteratorDirection*/

            /* specify iterator location relative to a particular b+ tree node */
            enum IteratorLocation {
                /*
                 * IL_BeforeBegin.    if non-empty tree, Iterator.node is the first node
                 *                    in the tree (the one with smallest key),
                 *                    and iterator refers to the location
                 *                    "one before" that first node.
                 * IL_Regular.        iterator refers to member of the tree
                 *                    given by Iterator.node
                 * IL_AfterEnd.       if non-empty tree, Iterator.node is the last node
                 *                    in the tree (the one with largest key),
                 *                    and iterator refers the the location
                 *                    "one after" that last node.
                 */
                IL_BeforeBegin,
                IL_Regular,
                IL_AfterEnd
            }; /*IteratorLocation*/

            static inline char const * iterator_location_descr(IteratorLocation x) {
                switch(x) {
                case IL_BeforeBegin: return "before-begin";
                case IL_Regular: return "regular";
                case IL_AfterEnd: return "after-end";
                default: return "???";
                }
            } /*iteerator_location_descr*/

            inline std::ostream &
            operator<<(std::ostream & os, IteratorLocation x) {
                os << iterator_location_descr(x);
                return os;
            } /*operator<<*/
        } /*namespace detail*/
    } /*namespace tree*/
} /*namespace xo*/

/* end IteratorUtil.hpp */
