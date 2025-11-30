/** @file RbTreeLhs.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "xo/indentlog/print/tostr.hpp"
#include <stdexcept>

namespace xo {
    namespace tree {
        namespace detail {
            /* xo::tree::detail::RedBlackTreeLhsBase
             *
             * use for const version of RedBlackTree::operator[].
             *
             * Require: RbNode is either
             *   RedBlackTree::RbNode
             * or
             *   RedBlackTree::RbNode const
             */
            template <class RedBlackTree, class RbNode>
            class RedBlackTreeLhsBase {
            public:
                using mapped_type = typename RedBlackTree::mapped_type;
                using RbUtil = typename RedBlackTree::RbUtil;

            public:
                RedBlackTreeLhsBase() = default;
                RedBlackTreeLhsBase(RedBlackTree * tree, RbNode * node)
                    : p_tree_(tree), node_(node)
                    {}

                operator mapped_type const & () const {
                    using xo::tostr;

                    if (!this->node_) {
                        throw std::runtime_error
                            (tostr("rbtree: attempt to use empty lhs object as rvalue"));
                    }

                    return this->node_->contents().second;
                } /*operator value_type const &*/

            protected:
                RedBlackTree * p_tree_ = nullptr;
                /* invariant: if non-nil, .node belongs to .*p_tree */
                RbNode * node_ = nullptr;
           }; /*RedBlackTreeLhsBase*/

            template<class RedBlackTree>
            class RedBlackTreeConstLhs : public RedBlackTreeLhsBase<RedBlackTree const,
                                                                    typename RedBlackTree::RbNode const>
            {
            public:
                RedBlackTreeConstLhs() = default;
                RedBlackTreeConstLhs(RedBlackTree const * tree,
                                     typename RedBlackTree::RbNode const * node)
                    : RedBlackTreeLhsBase<RedBlackTree const,
                                          typename RedBlackTree::RbNode const>(tree, node) {}
            }; /*RedBlackTreeConstLhs*/

            /* xo::tree::detail::RedBlackTreeLhs
             *
             * use for RedBlackTree::operator[].
             * can't return a regular lvalue,
             * because assignment within a Node N invalidates partial sums along
             * the path from tree root to N.
             *
             * instead interpolate instance of this class,   that can intercept
             * asasignments.
             */
            template <class RedBlackTree>
            class RedBlackTreeLhs : public RedBlackTreeLhsBase<RedBlackTree,
                                                               typename RedBlackTree::RbNode>
            {
            public:
                using value_type = typename RedBlackTree::value_type;
                using key_type = typename RedBlackTree::key_type;
                using mapped_type = typename RedBlackTree::mapped_type;
                using RbUtil = typename RedBlackTree::RbUtil;
                using RbNode = typename RedBlackTree::RbNode;

            public:
                RedBlackTreeLhs() = default;
                RedBlackTreeLhs(RedBlackTree * tree, typename RedBlackTree::RbNode * node, key_type key)
                    : RedBlackTreeLhsBase<RedBlackTree, RbNode>(tree, node), key_(key) {}

                RedBlackTreeLhs & operator=(mapped_type const & v) {
                    using xo::tostr;

                    constexpr bool c_debug_flag = false;

                    if(this->p_tree_) {
                        if(this->node_) {
                            this->node_->contents().second = v;

                            /* after modifying a node n,
                             * must recalculate reductions along path [root .. n]
                             */
                            RbUtil::fixup_ancestor_size(this->p_tree_->reduce_fn(),
                                                        this->node_,
                                                        c_debug_flag);
                        } else {
                            /* insert (key, v) pair into this tree */
                            this->p_tree_->insert(value_type(this->key_, v));
                        }
                    } else {
                        assert(false);

                        throw std::runtime_error
                            (tostr("rbtree: attempt to apply operator= thru empty lhs object"));
                    }

                    return *this;
                } /*operator=*/

                RedBlackTreeLhs & operator+=(mapped_type const & v) {
                    using xo::tostr;

                    if(this->p_tree_) {
                        if(this->node_) {
                            this->node_->contents().second += v;

                            /* after modifying value at node n,
                             * must recalculate order statistics along path [root .. n]
                             */
                            RbUtil::fixup_ancestor_size(this->p_tree_->reduce_fn(),
                                                        this->node_);
                        } else {
                            /* for form's sake,  in case value_type is something unusual */
                            mapped_type v2;
                            v2 += v;

                            /* insert (key, v) pair into this tree */
                            this->p_tree_->insert(value_type(this->key_, v2));
                        }
                    } else {
                        assert(false);

                        throw std::runtime_error
                            (tostr("rbtree: attempt to apply operator+= through empty lhs object"));
                    }

                    return *this;
                } /*operator+=*/

                /* TODO:
                 * - operator-=()
                 * - operator*=()
                 * - operator/=()
                 */

            private:
                /* capture key k used in expression tree[k]
                 * Invariant:
                 * - if .node is non-null,  then .node.key = key
                 */
                key_type key_;
            }; /*RedBlackTreeLhs*/
        } /*namespace detail*/
    } /*namespace tree*/
} /*namespace xo*/

/* end RbTreeLhs.hpp */
