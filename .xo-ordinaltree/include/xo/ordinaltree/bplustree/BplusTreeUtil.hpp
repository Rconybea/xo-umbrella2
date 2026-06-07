/* @file BplusTreeNode.hpp */

#pragma once

#include "IteratorUtil.hpp"
#include "bplustree_tags.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/tag.hpp"
#include <memory> // for std::unique_ptr
#include <string>

namespace xo {
    namespace tree {
        /* forward decl (see GenericNode.hpp) */
        template <typename Key, typename Value, typename Properties>
        class GenericNode;
        /* forward decl (see InternalNode.hpp) */
        template <typename Key, typename Value, typename Properties>
        class InternalNode;

        namespace detail {
            /* forward decl (see Iterator.hpp) */
            template <typename Key, typename Value, typename Properties>
            class ConstIterator;
        }

        // ----- NodeType -----

        enum class NodeType { internal, leaf };

        inline std::string node_type2str(NodeType x) {
            switch(x) {
            case NodeType::internal: return "internal";
            case NodeType::leaf: return "leaf";
            }

            return "???";
        } /*node_type2str*/

        inline std::ostream & operator<<(std::ostream & os, NodeType x) {
            os << node_type2str(x);
            return os;
        } /*operator<<*/

        /* see bplustree/LeafNode.hpp */
        template <typename Key, typename Value, typename Properties>
        struct LeafNode;

        /* see bplustree/InternalNode.hpp */
        template <typename Key, typename Value, typename Properties>
        struct InternalNode;

        // ----- NodeItem + NodeItemPlaceholder -----

        template <NodeType NType, typename Key, typename Value, typename Properties>
        struct NodeItem {};

        /* struct with same size as NodeItem<NType, Key, Value, Properties>,  but POD + with trivial ctor/dtor */
        template <NodeType NType, typename Key, typename Value, typename Properties>
        struct NodeItemPlaceholder {
            std::uint8_t mem_v_[sizeof(NodeItem<NType, Key, Value, Properties>)];
        }; /*NodeItemPlaceholder*/

        // ----- FindResult -----

        /* report a node,  along with its location (0-based index) within parent.
         * use nullptr for .node if item/node not found
         * use 0 for .ix if node is root (i.e. has no parent)
         *
         * expect ConcreteNodeType = LeafNode<..> | InternalNode<..>
         */
        template <typename ConcreteNodeType>
        struct FindNodeResult {
        public:
            FindNodeResult() = default;
            FindNodeResult(FindNodeResult const & x) = default;
            FindNodeResult(std::size_t ix, ConcreteNodeType * node) : ix_{ix}, node_{node} {}

            std::size_t ix() const { return ix_; }
            ConcreteNodeType * node() const { return node_; }

        private:
            /* 0-based index within parent */
            std::size_t ix_ = 0;
            /* a B+ tree node */
            ConcreteNodeType * node_ = nullptr;
        }; /*FindNodeResult*/

        template <typename Key,
                  typename Value,
                  typename Properties, tags::ordinal_tag OrdinalTag = Properties::ordinal_tag_value()>
        struct BplusTreeUtil {
        public:
            using GenericNodeType = GenericNode<Key, Value, Properties>;
            using InternalNodeType = InternalNode<Key, Value, Properties>;
            using LeafNodeType = LeafNode<Key, Value, Properties>;
            using const_iterator = detail::ConstIterator<Key, Value, Properties>;

            static std::size_t get_node_size(GenericNodeType const * node) {
                return node->size();
            }

            /* only implemented for OrdinalTag = ordinal_enabled */
            static void print_node_size(std::ostream & os, GenericNodeType const * node) {
                using xo::xtag;

                os << (node ? node->size() : 0UL);
            }

            static const_iterator find_ith(GenericNodeType * generic_node,
                                           std::size_t i_tree,
                                           const_iterator cend) {
                using xo::xtag;

                if (!generic_node)
                    return cend;

                std::size_t iter = 0;

                /* 100-level B+ tree won't fit in memory -- would have at least 2^100 nodes! */
                while (iter < 100) {
                    switch (generic_node->node_type()) {
                    case NodeType::leaf:
                        return const_iterator(detail::ID_Forward /*dirn*/,
                                              detail::IL_Regular /*loc*/,
                                              reinterpret_cast<LeafNodeType const *>(generic_node),
                                              i_tree /*item_ix*/);
                    case NodeType::internal:
                        {
                            /* scan for ith member (counting from 0) */

                            InternalNodeType const * internal_node
                                = reinterpret_cast<InternalNodeType const *>(generic_node);

                            std::size_t sum_z = 0;
                            std::size_t z = 0;

                            std::size_t i = 0;
                            std::size_t n = internal_node->n_elt();

                            for (; i<n; ++i) {
                                GenericNodeType * child_node = internal_node->lookup_elt(i).child();

                                z = child_node->size();

                                if (i_tree < sum_z + z) {
                                    /* continue search in i'th child of internal_node;
                                     * accounting for the sum_z members in nodes to the left of i_child
                                     */
                                    generic_node = child_node;
                                    i_tree = i_tree - sum_z;
                                    break;
                                }

                                sum_z += z;
                            }

                            if (i == n) {
                                throw std::runtime_error(tostr("BplusTree::find_ith: internal index failure",
                                                               xtag("i_tree", i_tree),
                                                               xtag("last_z", z),
                                                               xtag("n", internal_node->n_elt()),
                                                               xtag("sum_z", sum_z)));
                            }
                        }
                        break;
                    } /*switch*/

                    ++iter;
                } /*loop over descending internal node path*/

                throw std::runtime_error(tostr("BplusTree::find_ith: internal loop failure",
                                               xtag("iter", iter)));

                /* impossible! */
                return cend;
            } /*find_ith*/

            static void node_clear_size(InternalNodeType * node) {
                node->clear_size();
            }

            static void node_add_size(InternalNodeType * node, std::size_t incr_z) {
                node->add_size(incr_z);
            }

            static void node_sub_size(InternalNodeType * node, std::size_t decr_z) {
                node->sub_size(decr_z);
            }

            static void post_modify_add_ancestor_size(InternalNodeType * node, std::size_t incr_z, bool debug_flag) {
                using xo::scope;
                using xo::xtag;

                scope log(XO_DEBUG(debug_flag));

                while (node) {
                    log && log(xtag("node", node),
                               xtag("old_z", node->size()),
                               xtag("incr_z", incr_z));

                    node->add_size(incr_z);

                    node = node->parent();
                }
            } /*post_modify_add_ancestor_size*/

            static void post_modify_sub_ancestor_size(InternalNodeType * node, std::size_t decr_z, bool debug_flag) {
                using xo::scope;
                using xo::xtag;

                scope log(XO_DEBUG(debug_flag));

                while (node) {
                    log && log(xtag("node", node),
                               xtag("old_z", node->size()),
                               xtag("decr_z", decr_z));

                    node->sub_size(decr_z);

                    node = node->parent();
                }
            } /*post_modify_sub_ancestor_size*/
        };

        template <typename Key,
                  typename Value,
                  typename Properties>
        struct BplusTreeUtil<Key, Value, Properties, tags::ordinal_disabled> {
        public:
            using GenericNodeType = GenericNode<Key, Value, Properties>;
            using InternalNodeType = InternalNode<Key, Value, Properties>;
            using LeafNodeType = LeafNode<Key, Value, Properties>;
            using const_iterator = detail::ConstIterator<Key, Value, Properties>;

            static std::size_t get_node_size(GenericNodeType const * node) { return 0; }

            static void print_node_size(std::ostream & os, GenericNodeType const * node) {
                os << "n/a";
            }

            /* find_ith not implemented without ordinal feature */
            static const_iterator find_ith(GenericNodeType * generic_node,
                                           std::size_t i_tree,
                                           const_iterator cend) {
                throw std::runtime_error("BplusTreeUtil::find_ith: not implemented (requires tags::ordinal_enabled)");
            }

            /* per-node size not implemented,  so these are no-ops */
            static void node_clear_size(InternalNodeType * node) {}
            static void node_add_size(InternalNodeType * node, std::size_t incr_z) {}
            static void node_sub_size(InternalNodeType * node, std::size_t decr_z) {}
            static void post_modify_add_ancestor_size(InternalNodeType * node, std::size_t incr_z, bool debug_flag) {}
            static void post_modify_sub_ancestor_size(InternalNodeType * node, std::size_t decr_z, bool debug_flag) {}
        };
    } /*namespace tree*/
} /*namespace xo*/

namespace logutil {
    template <typename Node>
    struct nodesize {
        explicit nodesize(Node const * x) : node_{x} {}

        Node const * node() const { return node_; }

    private:
        Node const * node_ = nullptr;
    }; /*nodesize*/

    template <typename Node>
    inline std::ostream & operator<<(std::ostream & os,
                                     nodesize<Node> const & x) {
        xo::tree::BplusTreeUtil<typename Node::PropertiesType::KeyType,
                                typename Node::PropertiesType::ValueType,
                                typename Node::PropertiesType>::print_node_size(os, x.node());
        return os;
    };
} /*namespace logutil*/

/* end BplusTreeUtil.hpp */
