/* @file Lhs.hpp */

#pragma once

#include <stdexcept>

namespace xo {
    namespace tree {
        namespace detail {
            /* xo::tree::detail::BplusTreeLhsBase
             *
             * use for {const + non-const} versions of BplusTree::operator[]
             *
             * Expect: either:
             *   Tree         = BplusTree<Key, Value, Properties>
             *   LeafNodeItem = Tree::LeafNodeItemType
             * or
             *   Tree         = BplusTree<Key, Value, Properties> const
             *   LeafNodeItem = Tree::LeafNodeItemType const
             */
            template <class Tree, class LeafNodeItem>
            class BplusTreeLhsBase {
            public:
                using mapped_type = typename Tree::mapped_type;

            public:
                BplusTreeLhsBase() = default;
                BplusTreeLhsBase(Tree * tree, LeafNodeItem const * item)
                    : p_tree_{tree}, item_{item} {}

                operator mapped_type const & () const {
                    //using xo::tostr;

                    if (!this->item_) {
                        throw std::runtime_error
                            ("bptree: attempt to use empty lhs object as rvalue");
                    }

                    return this->item_->value();
                }

            protected:
                Tree * p_tree_ = nullptr;
                /* points to key-value pair (interior to a B+ tree LeafNode */
                LeafNodeItem * item_ = nullptr;
            }; /*BplusTreeLhsBase*/

            /* xo::tree::detail::BplusTreeConstLhs
             *
             * use for const version of BplusTree::operator[]
             */
            template <class BplusTree>
            class BplusTreeConstLhs : public BplusTreeLhsBase<BplusTree const,
                                                              typename BplusTree::LeafNodeItemType const>
            {
            public:
                BplusTreeConstLhs() = default;
                BplusTreeConstLhs(BplusTree const * tree,
                                  typename BplusTree::LeafNodeItemType const * item)
                    : BplusTreeLhsBase<BplusTree const,
                                       typename BplusTree::LeafNodeItemType const>(tree, item) {}
            }; /*BplusTreeConstLhs*/

        } /*namespace detail*/
    } /*namespace tree*/
} /*namespace xo*/

/* end Lhs.hpp */
