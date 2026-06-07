/* @file GenericNode.hpp */

#pragma once

#include "BplusTreeUtil.hpp"
#include "bplustree_tags.hpp"
#include <memory> // for std::unique_ptr
#include <string>

namespace xo {
    namespace tree {
        /* shim so we can partially specialize */
        template <typename Properties, tags::ordinal_tag OrdinalTag = Properties::ordinal_tag_value()>
        struct GenericNodeBase {
        }; /*GenericNodeBase*/

        template <typename Properties>
        struct GenericNodeBase<Properties, tags::ordinal_enabled> {
            /* #of items (key-value pairs) in this subtree */
            virtual std::size_t size() const = 0;
        }; /*GenericNodeShim*/

        // ----- GenericNode -----
        //
        // base class for LeafNode, InternalNode

        template <typename Key, typename Value, typename Properties>
        class GenericNode : public GenericNodeBase<Properties> {
        public:
            using PropertiesType = Properties;
            using InternalNodeType = InternalNode<Key, Value, Properties>;
            using LeafNodeType = LeafNode<Key, Value, Properties>;

        public:
            explicit GenericNode(NodeType ntype, std::size_t branching_factor)
                : node_type_{ntype}, branching_factor_{branching_factor} {}
            virtual ~GenericNode() = default;

            NodeType node_type() const { return node_type_; }
            InternalNodeType * parent() const { return parent_; }
            std::size_t n_elt() const { return n_elt_; }
            std::size_t branching_factor() const { return branching_factor_; }

            void set_parent(InternalNodeType * x) { this->parent_ = x; }

#ifdef OBSOLETE
            /* #of items (key-value pairs) in this subtree */
            virtual std::size_t size() const = 0;
#endif

            virtual Key const & glb_key() const = 0;
            /* support methods for BplusTree::verify()
             * with_lub.  true to use lub_key;  false to ignore
             * lub_key.   if with_lub=true, strict least upper bound key (in B+ tree) for this subtree;
             *            all keys in this subtree must be strictly less than lub_key.
             *            ignored when with_lub=false
             * lh_leaf.   if null, this subtree contains the smallest key in ancestor B+ tree;
             *            if non-null,  lh_leaf's rightmost key is immediate predecessor
             *            of leftmost key in this subtree
             * rh_leaf.   if null, this subtree contains the largest key in ancestor B+ tree;
             *            if non-null,  rh_leaf's leftmost key is immediate successor
             *            of rightmost key in this subtree
             */
            virtual std::size_t verify_helper(InternalNodeType const * parent,
                                              bool with_lub,
                                              Key const & lub_key,
                                              LeafNodeType const * lh_leaf,
                                              LeafNodeType const * rh_leaf) const = 0;
            virtual void verify_glb_key(Key const & key) const = 0;
            FindNodeResult<LeafNodeType const> c_find_min_leaf_node() const;
            FindNodeResult<LeafNodeType const> c_find_max_leaf_node() const;

            virtual FindNodeResult<LeafNodeType> find_min_leaf_node() = 0;
            virtual FindNodeResult<LeafNodeType> find_max_leaf_node() = 0;

            /* notification just before permanently removing this node from B+ tree */
            virtual void notify_remove() {}

        private:
            /* NodeType::internal | NodeType::leaf */
            NodeType node_type_;
            /* pointer to parent node
             * invariant: parent has direct pointer to this node,
             *            except briefly during construction
             */
            InternalNodeType * parent_ = nullptr;

        protected:
            /* #of non-empty elements (children) of this node
             *
             * invariant:
             * - .elt_v[i].child.ptr is non-null for 0 <= i < .n_elt
             * - for (0 < i < .n_elt):
             *   .elt_v[i-1].key < .elt_v[i].key
             * - elt_v[i].key not defined for (i >= .n_elt)
             */
            std::size_t n_elt_ = 0;
            /* need to store actual branching factor,  for LeafNode/InternalNode dtors */
            std::size_t branching_factor_ = 0;
        }; /*GenericNode*/

        /* const version (non-const version below) */
        template <typename Key, typename Value, typename Properties>
        FindNodeResult<LeafNode<Key, Value, Properties> const>
        GenericNode<Key, Value, Properties>::c_find_min_leaf_node() const {
            InternalNode<Key, Value, Properties> * self = const_cast<InternalNode<Key, Value, Properties> *>(this);

            return self->find_min_leaf_node();
        } /*c_find_min_leaf_node*/

        /* const version (non-const version below) */
        template <typename Key, typename Value, typename Properties>
        FindNodeResult<LeafNode<Key, Value, Properties> const>
        GenericNode<Key, Value, Properties>::c_find_max_leaf_node() const {
            InternalNode<Key, Value, Properties> * self = const_cast<InternalNode<Key, Value, Properties> *>(this);

            return self->find_max_leaf_node();
        } /*c_find_max_leaf_node*/

    } /*namespace tree*/
} /*namespace xo*/

/* end GenericNode.hpp */
