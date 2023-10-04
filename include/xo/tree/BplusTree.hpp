/* @file BplusTree.hpp */

/* provides B+ tree with order statistics */

/* NOTES:
 * - expect optimimum node size to be OS page size.
 *
 */

#pragma once

//#include "bplustree/BplusTreeNode.hpp"
#include "bplustree/LeafNode.hpp"
#include "bplustree/InternalNode.hpp"
#include "bplustree/Iterator.hpp"
#include "bplustree/Lhs.hpp"
#include "bplustree/bplustree_tags.hpp"
#include "indentlog/scope.hpp"
#include "indentlog/print/tag.hpp"
#include "indentlog/print/pad.hpp"
#include <memory> /* for std::unqiue_ptr */
#include <algorithm> /* for std::max */
#include <limits> /* for std::numeric_limits */
#include <cstdint>
#include <cassert>
#include <unistd.h>
#if __APPLE__ && __MACH__
# include <sys/sysctl.h>
#endif

namespace xo {
    namespace tree {
        /*
         *                                 +-------------+
         *                                 | BplusTree   |     +--------------------+
         *                                 | .properties +-----| BplusStdProperties |
         *                                 | .n_element  |     | .branching_factor  |
         *                                 | .root       |     | .debug_flag        |
         *                                 +------+------+     +--------------------+
         *                                        |
         *                                        | .root
         *                                        |
         *                           +-------------------+         +--------------+             +--------------+
         *                           | GenericNode       |   isa   | LeafNode     |  .elt_v[i]  | LeafNodeItem |
         *                           | .node_type        |<---+----| .elt_v[]     +-------------| .kv_pair     |
         *                           | .parent           |    |    |              |             |              |
         *                           | .n_elt            |    |    +--------------+             +--------------+
         *                           | .branching_factor |    |
         *                           +-------------------+    |
         *                                                    |
         *                                                    |    +--------------+             +------------------+
         *                                                    |    | InternalNode |  .elt_v[i]  | InternalNodeItem |
         *                                                    \----| .elt_v[]     +-------------| .key             |
         *                                                         |              |             | .child           |
         *                                                         +--------------+             +------------------+
         *
         * Invariants:
         * - tree is always balanced -- every path from root to a LeafNode, visits the same number of InternalNodes.
         * - all Nodes (both LeafNodes and InternalNodes) satisfy bf/2 <= .n_elt <= bf  (where bf = BplusTree.properties.branching_factor)
         * Details
         * - if InternalNode p has p.elt_v[i].child = q,  then q.parent = p
         * - GenericNode.branching_factor = BplusTree.properties.branching_factor for all nodes in the same BplusTree
         *
         * Tree with 0 key/value pairs
         *
         *                                                +--------------+
         *                                                | BplusTree    |
         *                                                | .root = null |
         *                                                +--------------+
         *
         * Tree with [1 .. b] key/value pairs (with b = BplusTree.properties.branching_factor)
         *
         *                                                +---------------+
         *                                                | BplusTree     |
         *                                                | .root = node1 |
         *                                                +--------+------+
         *                                                         |
         *                                 node1                   |
         *                                 +----------------------------------------------+
         *                                 | LeafNode                                     |
         *                                 | .parent = null                               |
         *                                 |                                              |
         *                                 | .elt_v[0]                .elt_v[b-1]         |   b = BplusTree.properties.branching_factor - 1
         *                                 | +----------------+- ... -+-----------------+ |   .elt_v[i].kv_pair.first = i'th  key
         *                                 | | k0     |    v0 |       | k(b-1) | v(b-1) | |   .elt_v[i].kv_pair.second = i'th value
         *                                 | +----------------+- ... -+-----------------+ |
         *                                 +----------------------------------------------+
         *
         * Tree with [b+1 ..] key/value pairs
         *
         *                                                +---------------+
         *                                                | BplusTree     |
         *                                                | .root = node1 |
         *                                                +--------+------+
         *                                                         |
         *                                 node1                   |
         *                                 +----------------------------------------------+
         *                                 | InternalNode                                 |
         *                                 | .parent = null                               |
         *                                 |                                              |
         *                                 | .elt_v[0]                .elt_v[b-1]         |
         *                                 | +----------------+- ... -+-----------------+ |   .elt_v[i].key = minimum key in subtree .elt_v[i].child
         *                                 | | k0     | node2 |       | k(b-1) | node(b)| |
         *                                 | +----------------+- ... -+-----------------+ |
         *                                 |  .key     .child          .key     .child    |
         *                                 +-------------------+-----+--------------------+
         *                                                     |     |
         *                                                     |     |  ...
         *                                                     |     |
         *                                    .elt_v[0].child  |     |    .elt_v[1].child
         *                                /--------------------/     \----------------------------\
         *                                |                                                       |
         *        node2                   |                             node3                     |
         *       +----------------------------------------------+      +-------------------------------------------------+
         *       | LeafNode                                     |      | LeafNode                                        |
         *       | .parent = node1                              |      | .parent = node1                                 |
         *       |                                              |      |                                                 |
         *       | .elt_v[0]                .elt_v[b-1]         |      | .elt_v[0]                 .elt_v[b-1]           |     .....
         *       | +----------------+- ... -+-----------------+ |      | +--------+--------+- ... -+---------+---------+ |
         *       | | k0     |    v0 |       | k(b-1) | v(b-1) | |      | | kb     | vb     |       | k(2b-1) | v(2b-1) | |
         *       | +----------------+- ... -+-----------------+ |      | +-----------------+- ... -+---------+---------+ |
         *       +----------------------------------------------+      +-------------------------------------------------+
         *
         *
         * Larger trees havedadditional levels comprising InternalNodes.
         *
         */

        /* NullReduce: 0-size reduce function (disappears at compile time) */
        template<typename NodeValue>
        struct NullReduce;

        struct Machdep {
            /* current page size (on a linux system).  Probably 4K
             *
             * (a) need this to be at least large enough to
             *     hold 3 keys
             * (b) note linux page size isn't fixed at compile time
             */
            static inline std::size_t get_page_size() {
                return ::sysconf(_SC_PAGESIZE);
            }

            /* L1 cache line size (on a linux system).  Probably 64 bytes */
            static inline std::size_t get_cache_line_size() {
                // https://sourceforge.net/p/predef/wiki/OperatingSystems/
#              if __APPLE__ && __MACH__
                std::size_t line_size = 0;
                std::size_t sizeof_line_size = sizeof(line_size);
                ::sysctlbyname("hw.cachelinesize",
                               &line_size, &sizeof_line_size, 0, 0);
                return line_size;
#              else
                return ::sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
#              endif
            }
        }; /*Machdep*/

        /* B+ tree nodes come in several flavors:  {root | internal | leaf}:
         *
         * - root.   each B+ tree has exactly one node of this type,
         *             representing the root of the B+ tree.
         *             The root node is subject to fewer restrictions than other
         *             nodes in a B+ tree:
         *             - can have 2..b elements (where b is branching factor for this B+ tree).
         *             - can function as tree's one and only leaf node (if tree has <= b items).
         *             - can function as an internal node (if tree has > b items)
         *
         * - internal. an internal node has:
         *             - n child node pointers,  subject to ceil(b/2) <= n <= b,
         *               where b is tree's branching factor
         *             - n keys.   key[j] is the smallest key value in subtree j.
         *             see InternalNode<Key, Properties>
         *
         * - leaf.    a leaf node has:
         *            - n keys,  subject to ceil(b/2) <= n <= b,
         *              where b is tree's branching factor
         *            - n values.   values are stored as pointers.
         *            - pointer to next leaf node,  to streamline inorder traversal
         */
        template <typename Key, typename Value, tags::ordinal_tag OrdinalTag = tags::ordinal_enabled>
        struct BplusStdProperties {
        public:
            using KeyType = Key;
            using ValueType = Value;

        public:
            BplusStdProperties() = default;
            explicit BplusStdProperties(std::size_t bf, bool debug_flag)
                : branching_factor_{bf}, debug_flag_{debug_flag} {}

            static constexpr tags::ordinal_tag ordinal_tag_value() { return OrdinalTag; }
            static constexpr bool ordinal_enabled() { return OrdinalTag == tags::ordinal_enabled; }

            static constexpr std::size_t c_min_branching_factor = 3;

            /* compute branching factor for given (leaf) node size */
            static constexpr std::size_t branching_factor_for_size(std::size_t z) {
                return std::max(c_min_branching_factor,
                                (z - sizeof(LeafNode<Key, Value, BplusStdProperties>))
                                / (sizeof(LeafNodeItemPlaceholder<Key, Value, BplusStdProperties>)));
            } /*branching_factor_for_size*/

            /* default branching factor.
             * attempt to optimize for cache efficiency of 'internal' nodes
             *
             * minimum branching factor always 3
             */
            static constexpr std::size_t default_branching_factor() {
                return branching_factor_for_size(Machdep::get_page_size());
            }

            /* expect this will be min branching factor
             * (i.e. smallest allowed LeafNode size likely won't fit in cache line):
             *
             * - cache line size = 64 bytes
             * - leaf node overhead = 56 bytes
             * - leaf node item size = 16 bytes
             */
            static constexpr std::size_t default_cacheline_branching_factor() {
                return branching_factor_for_size(Machdep::get_cache_line_size());
            }

            std::size_t branching_factor() const { return branching_factor_; }
            bool debug_flag() const { return debug_flag_; }

            void set_debug_flag(bool x) { debug_flag_ = x; }

        private:
            /* branching factor to use for both leaf an inteernal B+ tree nodes */
            std::size_t branching_factor_ = default_branching_factor();
            /* if true enable verbose logging during B+ tree operations */
            bool debug_flag_ = false;
        }; /*BplusStdProperties*/

        template <typename Key, typename Value, tags::ordinal_tag OrdinalTag>
        inline std::ostream &
        operator<<(std::ostream & os,
                   BplusStdProperties<Key, Value, OrdinalTag> const & p)
        {
            using xo::xtag;

            os << "<BplusStdProperties"
               << xtag("branching_factor", p.branching_factor())
               << ">";

            return os;
        } /*operator<<*/

        /* B+ tree with order statistics
         *
         * require:
         * - Key is equality comparable,  and imposes total ordering on keys.
         * - Key, Value, Reduce, Properties are copyable and null-constructible
         * - Reduce.value_type = Accumulator
         * - Reduce.operator() :: (Accumulator x Key) -> Accumulator
         */
        template <typename Key,
                  typename Value,
                  typename Reduce = NullReduce<Key>,
                  typename Properties = BplusStdProperties<Key, Value>>
        class BplusTree {
        public:
            using GenericNodeType = GenericNode<Key, Value, Properties>;
            using InternalNodeType = InternalNode<Key, Value, Properties>;
            using LeafNodeType = LeafNode<Key, Value, Properties>;
            using InternalNodeItemType = InternalNodeItem<Key, Value, Properties>;
            using LeafNodeItemType = LeafNodeItem<Key, Value, Properties>;
            using BpTreeConstLhs = detail::BplusTreeConstLhs<BplusTree<Key, Value, Reduce, Properties>>;
            using BpTreeUtil = BplusTreeUtil<Key, Value, Properties>;

            using key_type = Key;
            using mapped_type = Value;
            using value_type = std::pair<Key const, Value>;
            using size_type = std::size_t;
            using difference_type = std::ptrdiff_t;
            // key_compare
            // allocator_type
            using reference = value_type &;
            using const_reference = value_type const &;
            // pointer = std::allocator_traits<Allocator>::pointer;
            // const_pointer = std::allocator_traits<Allocator>::const_pointer;
            using const_iterator = detail::ConstIterator<Key, Value, Properties>;
            // reverse_iterator
            // const_reverse_iterator
            // value_compare (compares value_type objects by comparing their first elements)

        public:
            BplusTree() = default;
            explicit BplusTree(Properties const & properties) : properties_{properties} {}

            bool empty() const { return this->n_element_ == 0; }
            size_type size() const { return this->n_element_; }
            size_type max_size() const { return std::numeric_limits<difference_type>::max(); }
            std::size_t branching_factor() const { return this->properties_.branching_factor(); }

            bool debug_flag() const { return this->properties_.debug_flag(); }
            void set_debug_flag(bool x) { this->properties_.set_debug_flag(x); }

            /* verify b+ tree invariants.
             * if invariants satisfied,  return true.
             * if not satisfied,
             * - throw_flag=true -> throw execption
             * - throw_flag=false -> return false;
             */
            bool verify_ok(bool throw_flag = true) const {
                using xo::scope;
                using xo::xtag;

                //scope x("verify_ok");
                //x.log(xtag("n_element", this->n_element_));

                std::size_t z = 0;

                try {
                    z = this->verify_helper(throw_flag);
                } catch (...) {
                    if (throw_flag)
                        throw;

                    return false;
                }

                //x.log(xtag("z", z));

                if (z != this->n_element_) {
                    if (throw_flag) {
                        std::string err = tostr("BplusTree::verify_ok"
                                                ": bad key count",
                                                xtag("expected", this->n_element_),
                                                xtag("counted", z));

                        throw std::runtime_error(err);
                    }

                    return false;
                }

                return true;
            } /*verify_ok*/

            /* cxxx:  const iterator
             * rxxx:  reverse iterator
             * crxxx: const reverse iterator
             */

            const_iterator cprebegin() const { return const_iterator::prebegin_aux(this->leafnode_begin_); }
            const_iterator cbegin() const { return const_iterator::begin_aux(this->leafnode_begin_); }
            const_iterator cend() const { return const_iterator::end_aux(this->leafnode_end_); }

            const_iterator begin() const { return this->cbegin(); }
            const_iterator end() const { return this->cend(); }

            const_iterator crprebegin() const { return const_iterator::rprebegin_aux(this->leafnode_end_); }
            const_iterator crbegin() const { return const_iterator::rbegin_aux(this->leafnode_end_); }
            const_iterator crend() const { return const_iterator::rend_aux(this->leafnode_begin_); }

            const_iterator rbegin() const { return this->crbegin(); }
            const_iterator rend() const { return this->crend(); }

            /* find item with key equal to x in this tree.
             * success -> return iterator ix with ix->first = x
             * failure -> return iterator this->cend()
             */
            const_iterator find(Key const & x) const {
                FindNodeResult<LeafNodeType const> leaffindresult = this->find_leaf_node(x);
                LeafNodeType const * leaf = leaffindresult.node();

                std::pair<bool, std::size_t> lub_ix_recd = leaf->find_lub_ix(x);

                if (lub_ix_recd.first) {
                    return const_iterator(detail::ID_Forward /*dirn*/,
                                          detail::IL_Regular /*loc*/,
                                          leaf,
                                          lub_ix_recd.second - 1);
                } else {
                    return this->cend();
                }
            } /*find*/

            /* find i'th key/value pair (in key order) in this tree.
             *
             * Require:
             * - 0 <= i < .size
             */
            const_iterator find_ith(std::size_t i_tree) const {
                using xo::tostr;
                using xo::xtag;

                if (i_tree >= this->size()) {
                    throw std::runtime_error(tostr("BplusTree::find_ith: expected index i in range [0..n)",
                                                   xtag("i", i_tree),
                                                   xtag("n", this->size())));
                }

                GenericNodeType * generic_node = this->root_.get();

                return BplusTreeUtil<Key, Value, Properties>::find_ith(generic_node, i_tree, this->cend());
            } /*find_ith*/

            BpTreeConstLhs at(Key const & k) const {
                const_iterator ix = this->find(k);

                if (ix == this->cend()) {
                    throw std::out_of_range(tostr("BplusTree::at: expected key argument to appear in tree",
                                                  xtag("key", k)));
                }

                return BpTreeConstLhs(this, ix.item_addr());
            } /*at*/

            /* e.g.
             *   BplusTree<Key, ..> bptree = ...;
             *   Key key = ...;
             *   BplusTree::value_type x = bptree[key];
             */
            BpTreeConstLhs operator[](Key const & k) const {
                const_iterator ix = this->find(k);

                return BpTreeConstLhs(this, ix.item_addr());
            } /*operator[]*/

            void clear() {
                this->n_element_ = 0;
                this->leafnode_begin_ = nullptr;
                this->leafnode_end_ = nullptr;
                this->root_.reset(nullptr);
            } /*clear*/

            /* TODO:
             *   std::pair<iterator, bool> insert(value_type const & kv_pair);
             *
             *   template <typename P>
             *   std::pair<iterator, bool> insert(P && value)
             *
             *   std::pair<iterator, bool> insert(value_type && kv_pair);
             *
             *   iterator insert(iterator pos, value_type const & kv_pair);
             *   iterator insert(const_iterator pos, value_type const & kv_pair);
             *
             *   template <typename P>
             *   iterator insert(const_iterator pos, P && value);
             *
             *   iterator insert(const_iterator pos, value_type && value);
             *
             *   template <typename InputIterator>
             *   void insert(InputIterator lo, InputIterator hi);
             *
             *   void insert(std::initializer_list<value_type> initlist);
             */

            /* return:  true key already existed (tree size increases by 1)
             *          false if existing key (tree size unchanged)
             */
            std::pair<const_iterator, bool> insert(std::pair<Key const, Value> const & kv_pair) {
                using xo::scope;
                using xo::xtag;

                scope log(XO_DEBUG(this->debug_flag()),
                          xtag("key", kv_pair.first),
                          xtag("value", kv_pair.second),
                          xtag("root", this->root_.get())
                           //xtag("nesting", x.nesting_level())
                    );

                log && log(xtag("bptree[before-insert]", (char const *)"..."));
                if (log) this->print(std::clog, log.nesting_level()+2);

                std::pair<const_iterator, bool> retval;

                if (this->root_) {
                    NodeType root_type = this->root_->node_type();

                    log && log(xtag("root_type", root_type));

                    switch (root_type) {
                    case NodeType::leaf:
                        retval = this->leaf_insert_aux(kv_pair);
                        break;
                    case NodeType::internal:
                        retval = this->internal_insert_aux(kv_pair);
                        break;
                    } /*switch*/
                } else {
                    retval = this->create_root_aux(kv_pair);
                }

                log && log(xtag("bptree[after-insert]", (char const *)"..."));
                if (log) this->print(std::clog, log.nesting_level() + 2);

                log.end_scope();

                return retval;
            } /*insert*/

            /* e.g:
             *   std::map<A, B> m = ...;
             *   BplusTree<A, B> bptree;
             *
             *   bptree.insert(m.begin(), m.end());
             */
            template <typename InputIterator>
            void insert(InputIterator lo, InputIterator hi) {
                for (InputIterator ix = lo; ix != hi; ++ix)
                    this->insert(*ix);
            } /*insert*/

            /* return: true if key existed (tree size decreased by 1)
             *         false if key not found
             */
            bool erase(Key const & key) {
                using xo::scope;
                using xo::xtag;

                scope log(XO_DEBUG(this->debug_flag()),
                          xtag("key", key),
                          xtag("root", this->root_.get()));

                log && log(xtag("bptree[before-erase]", (char const *)"..."));
                if (log) this->print(std::clog, log.nesting_level()+2);

                bool retval = false;

                if (this->root_) {
                    NodeType root_type = this->root_->node_type();

                    log && log(xtag("root_type", root_type));

                    switch (root_type) {
                    case NodeType::leaf:
                        retval = leaf_erase_aux(key);
                        break;
                    case NodeType::internal:
                        retval = internal_erase_aux(key);
                        break;
                    } /*switch*/
                } else {
                    /* tree empty,  certainly doesn't contain key */
                }

                log && log(xtag("bptree[after-erase]", (char const *)"..."));
                if (log) this->print(std::clog, log.nesting_level()+2);

                log.end_scope();

                return retval;
            } /*erase*/

            void print(std::ostream & os, std::int32_t indent = 0) const {
                using xo::xtag;
                using xo::pad;

                os << pad(indent) << "<BplusTree";
                os << xtag("properties", properties_);
                os << xtag("n_element", n_element_);
                os << std::endl;

                os << pad(indent+1)
                   << xtag("root", (void*)root_.get())
                   << xtag("treez", logutil::nodesize(root_.get()));

                this->print_aux(os, this->root_.get(), indent+2);

                os << ">";
                os << std::endl;
            } /*print*/

        private:
            /* find leaf node associated with given key,  within given subtree
             *
             * .first:   index position of leaf in immediate parent of leaf node.  0 when leaf is also root node.
             * .seecond: leaf node
             */
            static FindNodeResult<LeafNodeType> find_leaf_node_aux(Key const & key, InternalNodeType * subtree_arg) {
                FindNodeResult<GenericNode<Key, Value, Properties>> findresult(0, subtree_arg);

                while (findresult.node() && (findresult.node()->node_type() == NodeType::internal)) {
                    findresult = (reinterpret_cast<InternalNodeType *>(findresult.node()))->find_child(key);
                }

                /* findresult.node().node_type() == NodeType::leaf (if non-null) */

                if (!findresult.node()) {
                    assert(false);
                    return FindNodeResult<LeafNodeType>();
                }

                assert(findresult.node()->node_type() == NodeType::leaf);

                /* subtree.canonical_node_type = leaf */
                return FindNodeResult<LeafNodeType>(findresult.ix(),
                                                reinterpret_cast<LeafNodeType *>(findresult.node()));
            } /*find_leaf_node_aux*/

            /* count #of keys present in this b+ tree,  by visiting every node;
             * but short-circuit if internal inconsistency detected
             */
            std::size_t verify_helper(bool throw_flag) const {
                using xo::scope;
                using xo::xtag;

                //scope x("BplusTree.verify_helper");

                if (Properties::ordinal_tag_value() == tags::ordinal_enabled) {
                    /* verify tree size (maintained in each node) matches toplevel tree size) */
                    if (this->root_ != nullptr) {
                        if (this->size() != BplusTreeUtil<Key, Value, Properties>::get_node_size(this->root_.get())) {
                            if (throw_flag) {
                                throw std::runtime_error(tostr("BplusTree::verify_helper"
                                                               ": mismatched tree size computation",
                                                               xtag("root", this->root_.get()),
                                                               xtag("bptree.n_element", this->size()),
                                                               xtag("bptree.root.size", logutil::nodesize(this->root_.get()))));
                            } else {
                                return -1;
                            }
                        }
                    }
                } else {
                    /* subtree size not maintained;  skip test */
                }

                /* verify leafnode iterator endpoints */

                if (this->root_ == nullptr) {
                    if (this->leafnode_begin_ != nullptr || this->leafnode_end_ != nullptr) {
                        if (throw_flag) {
                            throw std::runtime_error(tostr("BplusTree::verify_helper"
                                                           ": expected null .leafnode_begin / .leafnode_end pointers"
                                                           " with empty tree",
                                                           xtag("root", this->root_.get()),
                                                           xtag("leafnode_begin", this->leafnode_begin_),
                                                           xtag("leafnode_end", this->leafnode_end_)));
                        } else {
                            return -1;
                        }
                    }

                    return 0;
                } else {
                    auto leftmost_fr = this->root_->find_min_leaf_node();
                    auto rightmost_fr = this->root_->find_max_leaf_node();

                    if ((leftmost_fr.node() != this->leafnode_begin_)
                        || (rightmost_fr.node() != this->leafnode_end_))
                    {
                        if (throw_flag) {
                            throw std::runtime_error(tostr("BplusTree::verify_helper"
                                                           ": expected .leafnode_begin / .leafnode_end pointers"
                                                           " to match computed first/last leaf nodes",
                                                           xtag("root", this->root_.get()),
                                                           xtag("leafnode_begin[stored]", this->leafnode_begin_),
                                                           xtag("leafnode_begin[computed]", leftmost_fr.node()),
                                                           xtag("leafnode_end[stored]", this->leafnode_end_),
                                                           xtag("leafnode_end[computed]", rightmost_fr.node())));
                        } else {
                            return -1;
                        }
                    }
                }

                return this->root_->verify_helper(nullptr /*parent*/,
                                                  false /*!with_lub_flag*/,
                                                  Key() /*lub_key*/,
                                                  nullptr /*lh_leaf*/,
                                                  nullptr /*rh_leaf*/);
            } /*verify_helper*/

            /* find leaf node associated with given key;
             * this is the node that would contain target key,   if it is present.
             */
            FindNodeResult<LeafNodeType> find_leaf_node(Key const & key) {
                if (!root_.get())
                    return FindNodeResult<LeafNodeType>();

                switch (root_->node_type()) {
                case NodeType::leaf:
                    return FindNodeResult<LeafNodeType>(0, reinterpret_cast<LeafNodeType *>(root_.get()));
                case NodeType::internal:
                    return find_leaf_node_aux(key, reinterpret_cast<InternalNodeType *>(root_.get()));
                }

                assert(false);
                return FindNodeResult<LeafNodeType>();
            } /*find_leaf_node*/

            FindNodeResult<LeafNodeType const> find_leaf_node(Key const & key) const {
                FindNodeResult<LeafNodeType> findresult = const_cast<BplusTree *>(this)->find_leaf_node(key);

                return FindNodeResult<LeafNodeType const>(findresult.ix(), findresult.node());
            } /*find_leaf_node*/

            /* insert helper.
             *
             * require:
             * - root node is NodeType::leaf
             * - returns true if new key;  false if replace value associated with existing key
             */
            std::pair<const_iterator, bool>
            leaf_insert_aux(std::pair<Key const, Value> const & kv_pair) {
                using xo::scope;
                using xo::xtag;

                /* will add/replace key,value pair in existing root (which is a leaf) node */

                scope log(XO_DEBUG(this->debug_flag()),
                          xtag("key", kv_pair.first),
                          xtag("value", kv_pair.second));

                /* root node is a leaf node:
                 * - tree has between 1 and b elements (where b = branching factor)
                 */
                LeafNodeType * leaf = reinterpret_cast<LeafNodeType *>(this->root_.get());

                log && log(xtag("leaf", leaf),
                           xtag("leaf.n_elt", leaf->n_elt()),
                           xtag("leaf.bf", leaf->branching_factor()));

                /* .elt_v[]
                 *
                 *   0                   k           n-1      with: n <= b = branching factor
                 *   +---+---+- ... -+---+- ... -+---+---+          k = lub(key) in {e1..en}
                 *   | e1| e2|       | ek|       |   | en|
                 *   +---+---+- ... -+---+- ... -+---+---+
                 *
                 * lub_ix_recd.first:  true if key already present in tree.  implies lub_ix_recd.second >= 1
                 * lub_ix_recd.second: upper bound (strict) index position in .elt_v[] of key
                 */
                std::pair<bool, std::size_t> lub_ix_recd = leaf->find_lub_ix(kv_pair.first);

                log && log(xtag("lub_ix_recd.first", lub_ix_recd.first),
                           xtag("lub_ix_recd.second", lub_ix_recd.second));

                if (lub_ix_recd.first) {
                    leaf->assign_leaf_value(lub_ix_recd.second - 1, kv_pair.second);

                    return (std::pair<const_iterator, bool>
                            (const_iterator(detail::ID_Forward /*dirn*/,
                                            detail::IL_Regular /*loc*/,
                                            leaf,
                                            lub_ix_recd.second - 1),
                             false));
                }

                /* key not present in tree,  so will be incrementing tree size */

                if (leaf->n_elt() == leaf->branching_factor()) {
                    log && log("split root (leaf) node");

                    /* root node is full:
                     * 1. split into two leaf nodes
                     * 2. create new root node (internal instead of leaf)
                     */

                    ++(this->n_element_);

                    std::unique_ptr<LeafNodeType> lower(reinterpret_cast<LeafNodeType *>(this->root_.release()));
                    std::unique_ptr<LeafNodeType> upper(lower->split_leaf_upper());

                    /* insert is made into this leaf */
                    LeafNodeType * leaf_node = nullptr;
                    /* new (key, value) pair placed at this index poseition in leaf_node */
                    std::size_t leaf_ix = 0;

                    /* note corner case:
                     * when lub_ix_recd.second == lower->n_elt(),
                     * could either insert (key, value) as smallest key in upper subtree,  or largest key in lower subtree.
                     * however if we put into upper,  then also need to correct .root.elt_v_[1].key;
                     * slightly simpler to insert into lower subtree.
                     *
                     */
                    if (lub_ix_recd.second <= lower->n_elt()) {
                        log && log("insert into new LH leaf node");
                        leaf_node = lower.get();
                        leaf_ix = lub_ix_recd.second;
                    } else {
                        log && log("insert into new RH leaf node");
                        leaf_node = upper.get();
                        leaf_ix = lub_ix_recd.second - lower->n_elt();
                     }

                    leaf_node->insert_leaf_item(leaf_ix,
                                                std::move(kv_pair),
                                                this->debug_flag());

                    /* create new root node (now with node-type = internal),
                     * having two child (leaf) nodes
                     */
                    std::unique_ptr<InternalNodeType> root_2 = InternalNodeType::make_2(std::move(lower),
                                                                                        std::move(upper));

                    /* new root node replaces existing root node */
                    this->root_ = std::move(root_2);

                    this->post_modify_correct_leafnode_endpoints();

                    return (std::pair<const_iterator, bool>
                            (const_iterator(detail::ID_Forward /*dirn*/,
                                            detail::IL_Regular /*loc*/,
                                            leaf_node,
                                            leaf_ix),
                             true));
                } else if (leaf->n_elt() < this->properties_.branching_factor()) {
                    /* 1. key is not already present in b+ tree
                     * 2. leaf_ix+1 is lub on key;  move elements [lub .. n_elt) one step to the right
                     *     hope to move elements leaf_ix+1 .. n_elt-1 to the right,
                     */

                    /* leaf node has room for one more item */
                    ++(this->n_element_);
                    /* insert in root node, moving items [lub_ix .. .n_elt) one to the right */
                    leaf->insert_leaf_item(lub_ix_recd.second,
                                           kv_pair,
                                           this->debug_flag());

                    return (std::pair<const_iterator, bool>
                            (const_iterator(detail::ID_Forward /*dirn*/,
                                            detail::IL_Regular /*loc*/,
                                            leaf,
                                            lub_ix_recd.second),
                             true));
                } else {
                    /* impossible! */

                    assert(false);
                    return (std::pair<const_iterator, bool>
                            (const_iterator(),
                             false));
                }
            } /*leaf_insert_aux*/

            /* insert helper.
             *
             * require:
             * - root node is NodeType::internal
             *
             * kv_pair:    establish assocation kv_pair.first(=key) -> kv_pair.second(=value)
             * return:     true if insert performed;  false if update on existing key
             */
            std::pair<const_iterator, bool>
            internal_insert_aux(std::pair<Key const, Value> const & kv_pair) {
                using xo::scope;
                using xo::xtag;

                scope log(XO_DEBUG(this->debug_flag()),
                          xtag("key", kv_pair.first),
                          xtag("value", kv_pair.second));

                /* root node is an internal node:
                 * - tree has at least b elements (where b = branching factor)
                 */
                FindNodeResult<LeafNodeType> leaffindresult = this->find_leaf_node(kv_pair.first);
                LeafNodeType * leaf = leaffindresult.node();

                log && log(xtag("leaf", leaf),
                           xtag("leaf.n_elt", leaf->n_elt()),
                           xtag("leaf.bf", leaf->branching_factor()));

                std::pair<bool, std::size_t> lub_ix_recd = leaf->find_lub_ix(kv_pair.first);

                log && log(xtag("lub_ix_recd.first", lub_ix_recd.first),
                           xtag("lub_ix_recd.second", lub_ix_recd.second));

                if (lub_ix_recd.first) {
                    /* key already in tree,  just updating associated value */
                    leaf->assign_leaf_value(lub_ix_recd.second - 1, kv_pair.second);

                    return (std::pair<const_iterator, bool>
                            (const_iterator(detail::ID_Forward /*dirn*/,
                                            detail::IL_Regular /*loc*/,
                                            leaf,
                                            lub_ix_recd.second - 1),
                             false));
                }

                /* key not present in tree,  will be incrementing tree size */
                ++(this->n_element_);

                if (leaf->n_elt() < leaf->branching_factor()) {
                    log && log("insert into existing leaf,  since it has room");

                    /* leaf has room for 1 more item */
                    leaf->insert_leaf_item(lub_ix_recd.second,
                                           kv_pair,
                                           this->debug_flag());

                    this->post_modify_add_ancestor_size(leaf->parent(), +1);

                    /* whenever we insert at first key position,
                     * need also to update ancestor glb_key values
                     */
                    {
                        InternalNodeType * ancestor = leaf->parent();

                        while (ancestor && (kv_pair.first < ancestor->glb_key())) {
                            ancestor->set_glb_key(kv_pair.first);
                            ancestor = ancestor->parent();
                        }
                    }

                    return (std::pair<const_iterator, bool>
                            (const_iterator(detail::ID_Forward /*dirn*/,
                                            detail::IL_Regular /*loc*/,
                                            leaf,
                                            lub_ix_recd.second),
                             true));
                }

                /* leaf is full.
                 * 1. split into two half-full leaf nodes
                 * 2. insert new (key, value) pair into one of the two
                 *    half-full nodes.
                 * 3. recursively insert entry for new node into parent;
                 *    possibly splitting parent and additional ancestor
                 *    nodes as need be
                 */

                std::unique_ptr<GenericNodeType> new_node;

                /* key,value pair will be inserted into this leaf */
                LeafNodeType * leaf_node = nullptr;
                /* key,value pair inserted into leaf at this index position */
                std::size_t leaf_ix = 0;

                if (lub_ix_recd.second < leaf->n_elt() / 2) {
                    /* will insert into lower_leaf */
                    std::unique_ptr<LeafNodeType> lower_leaf(leaf->split_leaf_lower());

                    /* lower_leaf holds lower half of leaf's original set of items.
                     * leaf now holds upper half of leaf's original set of items.
                     */

                    log && log("split leaf to get lower_leaf",
                               xtag("lower_leaf", lower_leaf.get()),
                               xtag("leaf.n_elt", leaf->n_elt()),
                               xtag("lower_leaf.n_elt", lower_leaf->n_elt()));

                    assert(lub_ix_recd.second <= lower_leaf->n_elt());

                    /* this size temporarily excluded from tree */
                    std::size_t decr_z = lower_leaf->size();

                    log && log("insert new key into (new) LH leaf");
                    lower_leaf->insert_leaf_item(lub_ix_recd.second,
                                                 kv_pair,
                                                 this->debug_flag());

                    leaf_node = lower_leaf.get();
                    leaf_ix = lub_ix_recd.second;

                    new_node = std::move(lower_leaf);

                    /* new_node may get attached to ree at non-obvious location.
                     * at this point it is not in tree.
                     *
                     * to bookkeep node sizes,  decrement now, then increment where new_node is reintroduced
                     */
                    {
                        InternalNodeType * parent = leaf->parent();

                        BplusTreeUtil<Key, Value, Properties>::post_modify_sub_ancestor_size(parent, decr_z, this->debug_flag());
                    }

                    /* however:  leaf's glb increased ->
                     *           need to patch state in (at least parent, possibly more) ancestors
                     */
                    {
                        GenericNodeType * target = leaf;
                        InternalNodeType * parent = target->parent();

                        while (parent) {
                            std::size_t ix = parent->locate_child_by_address(target);

                            assert(ix != static_cast<std::size_t>(-1));

                            InternalNodeItemType & slot = parent->lookup_elt(ix);

                            if (slot.key() == target->glb_key()) {
                                /* done with fixup */
                                break;
                            }

                            slot.set_key(target->glb_key());

                            target = parent;
                            parent = parent->parent();
                        }
                    }
                } else {
                    /* leaf is full:
                     *
                     * note that leaf->n_elt() shrinks across this call
                     *
                     * before:
                     *   leaf:
                     *     <-- b elements ->
                     *     0   1       b-1
                     *     +---+- ... -+---+
                     *     | e1|       | eb|
                     *     +---+- ... -+---+
                     *
                     * after:
                     *   leaf:                      upper_leaf:
                     *     <-- b/2 elements ->        <-- b/2 elements -->
                     *     0   1       h              0
                     *     +---+- ... -+---+          +---+- ... -+---+
                     *     | e1|       | eh|          |eh'|       | eb|   with eh'=e(h+1)
                     *     +---+- ... -+---+          +---+- ... -+---+
                     *
                     *  note: if b odd,  then:
                     *    - leaf gets (b-1)/2 elements,
                     *    - upper_leaf gets (b+1)/2 elements
                     */

                    /* will insert into upper_leaf */
                    std::unique_ptr<LeafNodeType> upper_leaf(leaf->split_leaf_upper());

                    /* leaf now holds lower half of its original set of items;
                     * upper holds upper half of leaf's original set of items
                     */

                    log && log("split leaf to get upper_leaf",
                               xtag("upper_leaf", upper_leaf.get()),
                               xtag("leaf.n_elt", leaf->n_elt()),
                               xtag("upper_leaf.n_elt", upper_leaf->n_elt()));

                    assert(lub_ix_recd.second >= leaf->n_elt());

                    /* this size temporarily excluded from tree */
                    std::size_t decr_z = upper_leaf->size();

                    log && log("insert new key into (new) RH leaf");
                    upper_leaf->insert_leaf_item(lub_ix_recd.second - leaf->n_elt(),
                                                 kv_pair,
                                                 this->debug_flag());

                    leaf_node = upper_leaf.get();
                    leaf_ix = lub_ix_recd.second - leaf->n_elt();

                    new_node = std::move(upper_leaf);

                    /* new_node may get attached to tree at non-obvious location.
                     * at this point it is not in tree.
                     *
                     * to bookkeep node sizes,  decrement now, then increment where new_node is reintroduced
                     */
                    {
                        InternalNodeType * parent = leaf->parent();

                        BplusTreeUtil<Key, Value, Properties>::post_modify_sub_ancestor_size(parent, decr_z, this->debug_flag());
                    }

                    /* leaf's glb unchanged,  no glb fixup required here */
                }

                Key new_key = new_node->glb_key();
                std::size_t lub_ix = 0;

                InternalNodeType * ancestor = leaf->parent();

                while (ancestor) {
                    /* invariant: need to add new_node to tree somewhere on path to ancestor.
                     *            new_node is a leaf|internal node with already-correct size,
                     *            that isn't yet accounted for in this B+ tree
                     */

                    lub_ix = ancestor->find_lub_ix(new_key);

                    log && log("fixup ancestors",
                               xtag("new_key", new_key),
                               xtag("new_node", new_node.get()),
                               xtag("new_node.size", logutil::nodesize(new_node.get())),
                               xtag("ancestor", ancestor),
                               xtag("lub_ix", lub_ix));

                    /* on this iteration,  need to introduce (new_key, new_node) to ancestor */

                    if (ancestor->n_elt() < ancestor->branching_factor()) {
                        /* ordinal_enabled: #of elements in subtree new_node.
                         * ordinal_disabled: 0
                         */
                        std::size_t new_z = BplusTreeUtil<Key, Value, Properties>::get_node_size(new_node.get());

                        log && log("insert into ancestor, since it has room",
                                   xtag("ancestor.size[pre-insert]", logutil::nodesize(ancestor)),
                                   xtag("new_z", logutil::nodesize(new_node.get())));

                        /* room for 1 more child */
                        ancestor->insert_node(lub_ix,
                                              std::move(new_node),
                                              this->debug_flag());

                        /* if ordinal_enabled: increase .size on path from root down to and including ancestor
                         * otherwise no-op
                         */
                        BplusTreeUtil<Key, Value, Properties>::post_modify_add_ancestor_size(ancestor, new_z, this->debug_flag());
                        this->post_modify_correct_ancestor_glb_keys(ancestor);
                        this->post_modify_correct_leafnode_endpoints();

                        return (std::pair<const_iterator, bool>
                                (const_iterator(detail::ID_Forward /*dirn*/,
                                                detail::IL_Regular /*loc*/,
                                                leaf_node,
                                                leaf_ix),
                                 true));
                    } else {
                        log && log("pre-split (will split ancestor to make room for new node)",
                                   xtag("ancestor", ancestor),
                                   xtag("ancestor.size", logutil::nodesize(ancestor)),
                                   xtag("new_node", new_node.get()),
                                   xtag("new_node.size", logutil::nodesize(new_node.get())));

                        /* no room in ancestor,  need to split */

                        std::unique_ptr<InternalNodeType> upper_ancestor(ancestor->split_internal());

                        log && log("post-split",
                                   xtag("ancestor", ancestor),
                                   xtag("ancestor.size", logutil::nodesize(ancestor)),
                                   xtag("upper_ancestor", upper_ancestor.get()),
                                   xtag("upper_ancestor.size", logutil::nodesize(upper_ancestor.get())));

                        /* this size temporarily excluded from tree */
                        std::size_t decr_z = BplusTreeUtil<Key, Value, Properties>::get_node_size(upper_ancestor.get());

                        /* will add back size fom upper_ancestor (w/ +1 for insert),
                         * once we figure out where to attach it.
                         * ancestor.size already decreased via ancestor.split_internal()
                         */
                        BplusTreeUtil<Key, Value, Properties>::post_modify_sub_ancestor_size(ancestor->parent(), decr_z, this->debug_flag());

                        /* ancestor.n_elt reduced to 1/2 value before call to split_internal() */

                        std::size_t new_z = BplusTreeUtil<Key, Value, Properties>::get_node_size(new_node.get());

                        if (lub_ix <= ancestor->n_elt()) {
                            log && log("insert into (existing post-split) LH ancestor");
                            log && log(xtag("lub_ix", lub_ix), xtag("new_node", new_node.get()), xtag("new_z", new_z));

                            ancestor->insert_node(lub_ix,
                                                  std::move(new_node),
                                                  this->debug_flag());

                            /* note: updating entire ancestor chain,
                             *       since next iteration will operate on upper_ancestor != ancestor
                             */
                            BplusTreeUtil<Key, Value, Properties>::post_modify_add_ancestor_size(ancestor, new_z, this->debug_flag());

                            log && log("LH ancestor size",
                                       xtag("ancestor", ancestor),
                                       xtag("ancestor.size", logutil::nodesize(ancestor)));

                            /* note next loop iteration will fixup upper_ancestor.
                             * upper_ancestor != ancestor
                             */
                            this->post_modify_correct_ancestor_glb_keys(ancestor);
                        } else {
                            log && log("insert into (new) RH ancestor");
                            log && log(xtag("ix", lub_ix - ancestor->n_elt()),
                                       xtag("new_node", new_node.get()),
                                       xtag("new_z", new_z));

                            upper_ancestor->insert_node(lub_ix - ancestor->n_elt(),
                                                        std::move(new_node),
                                                        this->debug_flag());

                            /* note: deferring update for ancestor's ancestors until next loop iter */
                            BplusTreeUtil<Key, Value, Properties>::node_add_size(upper_ancestor.get(), new_z);

                            log && log("upper ancestor size",
                                       xtag("upper_ancestor", ancestor),
                                       xtag("upper_ancestor.size", logutil::nodesize(ancestor)));

                        }

                        /* setup for next loop iteration
                         * reminder: upper_ancestor.size was removed from computed treesize,
                         *           will add back on subsequent iteration (when attaching new_node)
                         */
                        new_key = upper_ancestor->glb_key();
                        new_node = std::move(upper_ancestor);
                        ancestor = ancestor->parent();
                    }
                }

                log && log("root node was split -> create new root,  adding one level");

                /* if control comes here:
                 * 1. ancestor is null
                 * 2. root node was full + has been split.  .
                 *      root will become LH subtree of new root
                 *      new_node will become RH subtree of new root
                 * 3. new_node is not present in .root
                 */

                log && log(xtag("root.n_elt", this->root_->n_elt()),
                           xtag("new_node.n_elt", new_node->n_elt()));

                this->root_ = std::move(InternalNodeType::make_2(std::move(this->root_),
                                                                 std::move(new_node)));

                this->post_modify_correct_leafnode_endpoints();

                return (std::pair<const_iterator, bool>
                        (const_iterator(detail::ID_Forward /*dirn*/,
                                        detail::IL_Regular /*loc*/,
                                        leaf_node,
                                        leaf_ix),
                            true));
            } /*internal_insert_aux*/

            std::pair<const_iterator, bool>
            create_root_aux(std::pair<Key const, Value> const & kv_pair) {
                /* create root,  with one element */
                this->n_element_ = 1;

                std::unique_ptr<LeafNodeType> leaf_node
                    = LeafNodeType::make(kv_pair,
                                         this->properties_);

                this->leafnode_begin_ = leaf_node.get();
                this->leafnode_end_ = leaf_node.get();

                std::pair<const_iterator, bool> retval
                    = (std::pair<const_iterator, bool>
                       (const_iterator(detail::ID_Forward /*dirn*/,
                                       detail::IL_Regular /*loc*/,
                                       leaf_node.get(),
                                       0),
                        true));

                this->root_.reset(leaf_node.release());

                return retval;
            } /*create_root_aux*/

            /* remove helper.
             *
             * require:
             * - root node is NodeType::leaf
             * - return true iff key found (in which case #of leaf node items decremented)
             */
            bool leaf_erase_aux(Key const & key) {
                using xo::scope;
                using xo::xtag;

                LeafNodeType * leaf = reinterpret_cast<LeafNodeType *>(this->root_.get());

                scope log(XO_DEBUG(this->debug_flag()),
                          xtag("leaf", leaf),
                          xtag("leaf.n_elt", leaf->n_elt()),
                          xtag("leaf.bf", leaf->branching_factor()));

                /* .elt_v[]
                 *
                 *   0                   k           n-1      with: n <= b = branching factor
                 *   +---+---+- ... -+---+- ... -+---+---+          k = lub(key) in {e1..en}
                 *   | e1| e2|       | ek|       |   | en|
                 *   +---+---+- ... -+---+- ... -+---+---+
                 *
                 * lub_ix_recd.first:  true if key already present in tree.  implies lub_ix_recd.second >= 1
                 * lub_ix_recd.second: upper bound (strict) index position in .elt_v[] of key
                 */
                std::pair<bool, std::size_t> lub_ix_recd = leaf->find_lub_ix(key);

                log && log(xtag("lub_ix_recd.first", lub_ix_recd.first),
                           xtag("lub_ix_recd.second", lub_ix_recd.second));

                if (!lub_ix_recd.first) {
                    /* key is not present in tree --> don't modify anything */
                    return false;
                }

                /* key is present in tree --> will decrement tree size */

                if (leaf->n_elt() > 1) {
                    --(this->n_element_);

                    /* reminder: lub_ix_recd.second is strict upper bound */
                    leaf->remove_leaf(lub_ix_recd.second - 1,
                                      this->debug_flag());

                } else {
                    --(this->n_element_);

                    /* removed last node -> tree now empty */

                    this->root_.reset();

                }

                this->post_modify_correct_leafnode_endpoints();

                log.end_scope();

                return true;
            } /*leaf_erase_aux*/

            /* remove helper.
             *
             * require:
             * - root node is NodeType::internal
             * - return true iff key found (in which case #of key,value pairs decremented)
             */
            bool internal_erase_aux(Key const & key) {
                using xo::scope;
                using xo::xtag;

                scope log(XO_DEBUG(this->debug_flag()),
                          xtag("key", key));

                std::size_t const bf = this->branching_factor();

                /* root node is an internal node:
                 * - tree has at least b elements (where b = branching zfactor)
                 *
                 *             this
                 *             +------+
                 *             |      |
                 *             +------+
                 *                 .
                 *                 .
                 *             +------+
                 *             |     i|    i = leaffindresult.ix()
                 *             +------+
                 *               / | \
                 *      /-------/  |  \--------\
                 *      |          |           |
                 *  +------+   +------+     +------+
                 *  |      |   |      |     |    j |    j = lub_ix_recd.second - 1
                 *  +------+   +------+     +------+
                 *                          leaf
                 */

                FindNodeResult<LeafNodeType> leaffindresult = this->find_leaf_node(key);
                LeafNodeType * leaf = leaffindresult.node();

                log && log(xtag("leaf", leaffindresult.node()),
                           xtag("leaf.n_elt", leaffindresult.node()->n_elt()),
                           xtag("leaf.loc", leaffindresult.ix()),
                           xtag("bf", bf));

                std::pair<bool, std::size_t> lub_ix_recd = leaffindresult.node()->find_lub_ix(key);

                log && log(xtag("lub_ix_recd.first", lub_ix_recd.first),
                           xtag("lub_ix_recd.second", lub_ix_recd.second));

                if (!lub_ix_recd.first) {
                    /* key not in tree */
                    return false;
                }

                /* key present in tree at leaf.elt_v[lub_ix_recd.second - 1] */

                /* B+ balance invariant is sustained across remove
                 *
                 * if glb key changed,  then have to propagate up ancestor chain
                 */
                --(this->n_element_);

                /* reminder: lub_ix_recd.second is strict upper bound */
                leaf->remove_leaf(lub_ix_recd.second - 1,
                                  this->debug_flag());

                InternalNodeType * parent = leaf->parent();

                /* whenever we remove at first key position (with strict upper bound index 1),
                 * then glb key changed,  so need also to update ancestor glb_key values
                 */
                if (lub_ix_recd.second == 1) {
                    /* glb_key for this leaf node changed (to larger value) */
                    log && log("fix glb",
                               xtag("@", parent),
                               xtag("old-glb", parent->glb_key()),
                               xtag("new-glb", leaf->glb_key()));

                    /* we dropped smallest key from [leaf] --> correct glb key for leaf in its immediate parent */
                    parent->lookup_elt(leaffindresult.ix()).set_key(leaf->glb_key());

                    this->post_modify_correct_ancestor_glb_keys(parent);
                } else {
                    /* removal from position >0 doesn't change glb key
                     *   -> doesn't require ancestor updates
                     */
                }

                if (2 * leaf->n_elt() >= bf) {
                    /* after removal, leaf still has acceptable #of children */

                    /* must decrement tree size on path from root down to and including parent */
                    this->post_modify_sub_ancestor_size(parent, +1);

                    return true;
                } else {
                    /* after removal, leaf will be too small.  plan:
                     * - try redistributing from a neighboring leaf
                     * - if result too small,  then merge with one of neighboring leaves;
                     *   in this case merges may cascade upward to root
                     * - if root node shrinks to 1 child,  that child becomes new root
                     */
                    log && log("leaf too small after remove -> redistribute or shrink tree");

                    std::size_t leaf_ix = leaffindresult.ix();
                    /* right_sibling_ix: position of sibling immediately after (leaf_ix, key), in parent */
                    std::size_t right_sibling_ix = leaf_ix + 1;

                    LeafNodeType * right_sibling = nullptr;

                    if (right_sibling_ix < parent->n_elt()) {
                        /* consider merge with right sibling */
                        right_sibling = reinterpret_cast<LeafNodeType *>(parent->lookup_elt(right_sibling_ix).child());

                        std::size_t n = leaf->n_elt() + right_sibling->n_elt();

                        if (n >= 2 * ((bf + 1) / 2)) {
                            /* can redistribute one or more nodes from right_sibling -> leaf
                             * e.g.
                             *    if bf=3,  require 4 nodes between leaf and rh sibling
                             *    if bf=4,  also require 4 nodes between leaf and rh sibling.
                             *
                             * after redistribution:
                             * - leaf will have n/2 elements
                             * - right_sibling will have n - n/2 elements
                             */
                            leaf->append_from_rh_sibling(n/2 - leaf->n_elt(), right_sibling);

                            /* glb_key for right sibling changed,  need to fix ancestor book-keeping */
                            parent->lookup_elt(right_sibling_ix).set_key(right_sibling->glb_key());

                            this->post_modify_sub_ancestor_size(parent, +1);
                            this->post_modify_correct_ancestor_glb_keys(parent);

                            return true;
                        } else {
                            log && log("reject redistrib from right sibling,  not enough capacity");
                        }
                    } else {
                        log && log("reject redistrib from right sibling,  doesn't exist");
                    }

                    std::size_t left_sibling_ix = leaf_ix - 1;
                    LeafNodeType * left_sibling = nullptr;

                    if (leaf_ix > 0) {
                        /* consider redistribution from left sibling */
                        left_sibling = reinterpret_cast<LeafNodeType *>(parent->lookup_elt(left_sibling_ix).child());

                        std::size_t n = leaf->n_elt() + left_sibling->n_elt();

                        if (n >= 2 * ((bf + 1) / 2)) {
                            log && log("redistrib from left sibling");

                            std::size_t n_redistrib = n/2 - leaf->n_elt();

                            log && log(xtag("n/2", n/2),
                                       xtag("leaf.n", leaf->n_elt()),
                                       xtag("n_redistrib", n_redistrib));

                            /* can redistribute one or more nodes from left_sibling -> leaf
                             * after redistribution:
                             * - leaf will have n/2 elements
                             * - left_sibling will have n - n/2 elements
                             */
                            leaf->prepend_from_lh_sibling(left_sibling, n_redistrib, this->debug_flag());

                            /* glb key for leaf changed,  need to fix ancestor book-keeping */
                            parent->lookup_elt(leaf_ix).set_key(leaf->glb_key());

                            this->post_modify_sub_ancestor_size(parent, +1);
                            this->post_modify_correct_ancestor_glb_keys(parent);

                            return true;
                        } else {
                            log && log("reject redistib from left sibling,  not enough capacity");
                        }
                    } else {
                        log && log("reject redistrib from left sibling,  doesn't exist");
                    }

                    /* control here
                     *   -> not enough nodes to redistribute from either sibling
                     *   -> must shrink #nodes in tree
                     */

                    if (right_sibling_ix < parent->n_elt()) {
                        assert(right_sibling);

                        log && log("merge right sibling");

                        /* RH sibling exists -> merge with it (arbitrary choice if leaf_ix > 0) */

                        leaf->append_rh_sibling(right_sibling);

                        /* right_sibling is now (effectively) empty,  drop from parent;
                         * also fixup next_leafnode/prev_leafnode links to bypass
                         */
                        parent->remove_node(right_sibling_ix, this->debug_flag());

                        /* note that glb_key for leaf did not change */

                        /* -1 .size on path from root down to and including parent */
                        this->post_modify_sub_ancestor_size(parent, +1);
                        /* since we reduced #of children at parent,  it may have fallen below b/2 lower bound */
                        this->post_remove_shrink_ancestor_path(parent);
                        /* since we removed a leaf node,  may have invalidated iterator begin/end endpoints */
                        this->post_modify_correct_leafnode_endpoints();

                        return true;
                    }

                    if (leaf_ix > 0) {
                        assert(left_sibling);

                        /* LH sibling exists -> merge with it (arbitrary choice if right_sibling_ix < parent.n_elt */

                        left_sibling->append_rh_sibling(leaf);

                        /* leaf is now (effectively) empty,  drop from parent;
                         * also fixup next_leafnode/prev_leafnode links to bypass
                         */
                        parent->remove_node(leaf_ix, this->debug_flag());

                        /* note that glb_key for left_sibling did not change */

                        this->post_modify_sub_ancestor_size(parent, +1);
                        /* since we reduced #of children at parent,  it may have fallen below b/2 lower bound */
                        this->post_remove_shrink_ancestor_path(parent);
                        /* since we removed a leaf node,  may have invalidated iterator begin/end endpoints */
                        this->post_modify_correct_leafnode_endpoints();

                        return true;
                    }

                    /* must have at least one sibling (else prior visit would have shrunk tree height) */
                }

                log.end_scope();

                assert(false);
                return false;
            } /*internal_erase_aux*/

            void post_modify_add_ancestor_size(InternalNodeType * parent, std::size_t incr_z) {
                BplusTreeUtil<Key, Value, Properties>::post_modify_add_ancestor_size(parent, incr_z, this->debug_flag());
            } /*post_modify_add_ancestor_size*/

            void post_modify_sub_ancestor_size(InternalNodeType * parent, std::size_t decr_z) {
                BplusTreeUtil<Key, Value, Properties>::post_modify_sub_ancestor_size(parent, decr_z, this->debug_flag());
            } /*post_modify_sub_ancestor_size*/

            void post_modify_correct_ancestor_glb_keys(InternalNodeType * parent) {
                using xo::scope;
                using xo::xtag;

                scope log(XO_DEBUG(this->debug_flag()),
                          xtag("parent", parent));

                InternalNodeType * grandparent = parent->parent();

                std::size_t i_ancestor = 0;;
                while (grandparent) {
                    log && log(xtag("i_ancestor", i_ancestor),
                               xtag("grandparent", grandparent));

                    /* find index position of parent subtree,  as child of grandparent
                     * Can only use .find_ix() when key-invariants are satisfied.
                     *
                     * Warning: O(bf) call here
                     */
                    std::size_t parent_ix = grandparent->locate_child_by_address(parent);

                    log && log(xtag("parent.loc", parent_ix));

                    if (grandparent->lookup_elt(parent_ix).key() == parent->glb_key()) {
                        log && log("grandparent[parent.loc].key == parent.glb_key --> done");
                        break;
                    }

                    log && log("fix glb key in grandparent");
                    grandparent->lookup_elt(parent_ix).set_key(parent->glb_key());

                    /* + repeat 1 level up.. */
                    parent = grandparent;
                    grandparent = parent->parent();
                }
            } /*post_modify_correct_ancestor_glb_keys*/

            /* reset .leafnode_begin, .leafnode_end after changing the set of nodes in a b+ tree */
            void post_modify_correct_leafnode_endpoints() {
                if (root_) {
                    this->leafnode_begin_ = root_->find_min_leaf_node().node();
                    this->leafnode_end_ = root_->find_max_leaf_node().node();
                } else {
                    this->leafnode_begin_ = nullptr;
                    this->leafnode_end_ = nullptr;
                }
            } /*post_modify_correct_leafnode_endpoints*/

            void post_remove_shrink_ancestor_path(InternalNodeType * node) {
                using xo::scope;
                using xo::xtag;

                scope log(XO_DEBUG(this->debug_flag()));

                std::size_t const bf = node->branching_factor();

                while (node
                       && (node != this->root_.get())) {

                    log && log(xtag("node", node),
                               xtag("node.n_elt", node->n_elt()));

                    if (2 * node->n_elt() >= bf)
                        break;

                    /* node has fewer children than B+ minimum.
                     * either:
                     * - redistribute nodes from sibling
                     *   (so that merged node satisfies bf/2 <= n <= bf)
                     * - merge with sibling
                     */
                    InternalNodeType * parent = node->parent();

                    /* O(bf),  but doesn't rely on satisfied key invariants */
                    std::size_t node_ix = parent->locate_child_by_address(node);
                    std::size_t right_sibling_ix = node_ix + 1;

                    InternalNodeType * right_sibling = nullptr;

                    if (right_sibling_ix < parent->n_elt()) {
                        /* consider redistributng from right sibling */
                        right_sibling = reinterpret_cast<InternalNodeType *>(parent->lookup_elt(right_sibling_ix).child());

                        std::size_t n = node->n_elt() + right_sibling->n_elt();

                        if (n >= 2 * ((bf + 1) / 2)) {
                            log && log("redistribute from right_sibling",
                                       xtag("lh.n", node->n_elt()),
                                       xtag("rh.n", right_sibling->n_elt()));

                            /* can redistribute one or more nodes from right_sibling -> node
                             *
                             * after redistribution:
                             * - node will have floor(n/2) elements
                             * - right_sibling will have ceil(n/2) = n - floor(n/2) elements
                             */
                            node->append_from_rh_sibling(n/2 - node->n_elt(), right_sibling);

                            /* glb_key for right sibling changed,  need to fixup ancestor book-keeping */
                            this->post_modify_correct_ancestor_glb_keys(right_sibling);

                            return;
                        }
                    }

                    std::size_t left_sibling_ix = node_ix - 1;   /* but beware underflow when node_ix=0 */

                    InternalNodeType * left_sibling = nullptr;

                    if (node_ix > 0) {
                        /* consider redistributing from left sibling */
                        left_sibling = reinterpret_cast<InternalNodeType *>(parent->lookup_elt(left_sibling_ix).child());

                        std::size_t n = node->n_elt() + left_sibling->n_elt();

                        if (n >= 2 * ((bf + 1) / 2)) {
                            log && log("redistribute from left_sibling",
                                       xtag("lh.n", left_sibling->n_elt()),
                                       xtag("rh.n", node->n_elt()));

                            /* redistribute one or more nodes from left_sibling -> node */
                            node->prepend_from_lh_sibling(left_sibling,
                                                          n/2 - node->n_elt(),
                                                          this->debug_flag());

                            /* glb_key for node changed,  need to fixup ancestor book-keeping */
                            this->post_modify_correct_ancestor_glb_keys(node);

                            return;
                        }
                    }

                    log && log("cannot redistribute -> drop a node");

                    /* control here
                     *   -> not enough nodes to redistribute from either sibling
                     *   -> must shrink number of nodes in tree
                     */

                    if (right_sibling_ix < parent->n_elt()) {
                        assert(right_sibling);

                        /* RH sibling exists -> merge with it */

                        node->append_rh_sibling(right_sibling);

                        /* right sibling now empty,  drop from parent */
                        parent->remove_node(right_sibling_ix, this->debug_flag());
                    } else if (node_ix > 0) {
                        assert(left_sibling);

                        /* LH sibling exists -> merge with it */

                        left_sibling->append_rh_sibling(node);

                        /* node is now empty,  drop from parent */
                        parent->remove_node(node_ix, this->debug_flag());
                    }

                    /* continue tree fixup at parent node */
                    node = parent;
                }

                /* if node != root:  tree shrank successfully,  without propgating to root */

                if ((node == this->root_.get()) && (node->n_elt() == 1))
                {
                    /* replace root with its single child element;  tree height shrinks by one */
                    this->root_ = std::move(node->lookup_elt(0).release_child());

                    this->root_->set_parent(nullptr);
                }
            } /*post_remove_shrink_ancestor_path*/

            void print_aux(std::ostream & os,
                           GenericNodeType const * node,
                           std::uint32_t indent) const
            {
                using xo::xtag;

                if (node) {
                    switch(node->node_type()) {
                    case NodeType::internal:
                        {
                            using xo::pad;

                            InternalNodeType const * internal = reinterpret_cast<InternalNodeType const *>(node);

                            for (std::uint32_t i=0, n=internal->n_elt(); i<n; ++i) {
                                os << std::endl
                                   << pad(indent)
                                   << ":child " << i << "/" << n
                                   << xtag("n", internal->lookup_elt(i).child()->n_elt())
                                   << xtag("treez", logutil::nodesize(internal->lookup_elt(i).child()))
                                   << xtag("glb", internal->lookup_elt(i).key())
                                   << xtag("@", internal->lookup_elt(i).child());

                                this->print_aux(os,
                                                internal->lookup_elt(i).child(),
                                                indent+1);
                            }
                        }
                        break;
                    case NodeType::leaf:
                        {
                            using xo::pad;

                            LeafNodeType const * leaf = reinterpret_cast<LeafNodeType const *>(node);

                            for (std::uint32_t i=0, n=leaf->n_elt(); i<n; ++i) {
                                os << std::endl
                                   << pad(indent)
                                   << leaf->lookup_elt(i).key()
                                   << ": " << leaf->lookup_elt(i).value();
                            }
                        }
                        break;
                    }
                } else {
                    //os << std::endl;
                }
            } /*print_aux*/

        private:
            /* tree properties,  in particular: branching factor */
            Properties properties_;

            /* #of items in this tree */
            std::size_t n_element_ = 0;

            /* left-most leaf node for inorder traversal */
            LeafNodeType * leafnode_begin_ = nullptr;
            /* right-most leaf node for inorder traversal */
            LeafNodeType * leafnode_end_ = nullptr;

            /* tree
             * size          root  depth
             * -------------------------
             *    0       nullptr      0
             * 1..b      LeafNode      1
             *   >b  InternalNode     >1
             */
            std::unique_ptr<GenericNodeType> root_;
        }; /*BplusTree*/

    } /*namespace tree*/
} /*namespace xo*/

/* end BplusTree.hpp */
