/* @file InternalNode.hpp */

#pragma once

#include "GenericNode.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/tostr.hpp"
#include <cassert>

namespace xo {
    namespace tree {
        // ----- InternalNodeItem ------

        /* see also: NodeItem<NodeType::leaf, Key, Value, Properties> */
        template <typename Key, typename Value, typename Properties>
        struct NodeItem<NodeType::internal, Key, Value, Properties> {
            using GenericNodeType = GenericNode<Key, Value, Properties>;

        public:
            NodeItem() = default;
            explicit NodeItem(std::unique_ptr<GenericNodeType> child)
                : child_{std::move(child)} {
                if (child_)
                    this->key_ = child_->glb_key();
            }

            Key const & key() const { return key_; }
            GenericNodeType * child() const { return child_.get(); }

            std::unique_ptr<GenericNodeType> release_child() { return std::move(child_); }

            void set_key(Key key) { key_ = std::move(key); }

            void notify_remove() {
                if (child_)
                    child_->notify_remove();
            } /*notify_remove*/

        private:
            /* invariant: .key is leftmost key in subtree rooted at .child
             *            (i.e. greatest lower bound for keys in that subtree)
             */
            Key key_;
            /* subtree.  subtree has minimum key value .key */
            std::unique_ptr<GenericNodeType> child_;
        }; /*NodeItem */

        template <typename Key, typename Value, typename Properties>
        using InternalNodeItem = NodeItem<NodeType::internal, Key, Value, Properties>;

        /* struct with same size as InternalNodeItem<Key,Properties>,  but POD + with no ctor/dtor */
        template <typename Key, typename Value, typename Properties>
        using InternalNodeItemPlaceholder = NodeItemPlaceholder<NodeType::internal, Key, Value, Properties>;

        /* default implements tags::ordinal_disabled;  see partial specialization below for ordinal_enabled */
        template <typename Key, typename Value, typename Properties, tags::ordinal_tag OrdinalTag = Properties::ordinal_tag_value()>
        struct InternalNodeShim : public GenericNode<Key, Value, Properties> {
        public:
            using GenericNodeType = GenericNode<Key, Value, Properties>;

        public:
            InternalNodeShim(NodeType ntype, std::size_t branching_factor) : GenericNode<Key, Value, Properties>{ntype, branching_factor} {}

        protected:
            /* not implemented with tags::ordinal_disabled */
            void assign_size(std::size_t z) {}
        };

        template <typename Key, typename Value, typename Properties>
        struct InternalNodeShim<Key, Value, Properties, tags::ordinal_enabled> : public GenericNode<Key, Value, Properties> {
        public:
            using GenericNodeType = GenericNode<Key, Value, Properties>;

        public:
            InternalNodeShim(NodeType ntype, std::size_t branching_factor) : GenericNode<Key, Value, Properties>{ntype, branching_factor} {}

            void clear_size() { this->size_ = 0; }
            void add_size(std::size_t z) { this->size_ += z; }
            void sub_size(std::size_t z) { this->size_ -= z; }

            virtual std::size_t size() const override { return size_; }

        protected:
            void assign_size(std::size_t z) { this->size_ = z; }

        protected:
            std::size_t size_ = 0;
        }; /*InternalNodeShim*/

        /* require:
         * - Properties.branching_factor()
         */
        template <typename Key, typename Value, typename Properties>
        struct InternalNode : public InternalNodeShim<Key, Value, Properties> {
        public:
            using GenericNodeType = GenericNode<Key, Value, Properties>;
            using InternalNodeType = InternalNode<Key, Value, Properties>;
            using LeafNodeType = LeafNode<Key, Value, Properties>;
            using InternalNodeItemPlaceholderType = InternalNodeItemPlaceholder<Key, Value, Properties>;
            using InternalNodeItemType = InternalNodeItem<Key, Value, Properties>;

        public:
            virtual ~InternalNode();

            /* node size in bytes (increases with branching factor) */
            static std::size_t node_sizeof(std::size_t branching_factor);

            /* use when splitting root node for the first time;
             * new root node will be leaf->internal.
             *
             * require: child_1, child_2 are non-empty
             */
            static std::unique_ptr<InternalNode> make_2(std::unique_ptr<GenericNodeType> child_1,
                                                        std::unique_ptr<GenericNodeType> child_2);

            /* Before:
             *
             *   m = mid_ix
             *   n = src.n_elt - 1
             *   xa @ [m-1]
             *   xb @ [m]
             *   xz @ [n-1]
             *
             *  src.elt_v[]
             *
             *        0        m-1    m        n-1
             *    +----+-...-+----+----+-...-+----+
             *    | x0 | ... | xa | xb | ... | xz |
             *    +----+-...-+----+----+-...-+----+
             *
             *    <----------- n items ----------->
             *
             * After:
             *
             *  src.elt_v[]               new_node.elt_v[]
             *
             *                                    n-m-1
             *        0        m-1         0          v
             *    +----+-...-+----+        +----+-...-+----+
             *    | x0 | ... | xa |        | xb |     | xz |
             *    +----+-...-+----+        +----+-...-+----+
             *
             *    <--- m items --->        <-- n-m items -->
             */
            static std::unique_ptr<InternalNode> annex(std::size_t mid_ix,
                                                       InternalNode * src);

            /* .elt_v[]
             *
             *   0                   k           n-1      with: n <= b = branching factor
             *   +---+---+- ... -+---+- ... -+---+---+          k = lub(key) in {e1..en}
             *   | e1| e2|       | ek|       |   | en|
             *   +---+---+- ... -+---+- ... -+---+---+
             *
             * retval.first:  true if key already present in tree.  implies lub_ix_recd.second >= 1
             * retval.second: upper bound (strict) index position in .elt_v[] of key
             *
             * Cost: O(log(bf)) key comparisons
             */
            std::size_t find_lub_ix(Key const & key) const;

            /* warning:  requires key is present! */
            std::size_t find_ix(Key const & key) const { return this->find_lub_ix(key) - 1; }

            /* O(bf),  but does not rely on key invariants. */
            std::size_t locate_child_by_address(GenericNodeType const * target_child) const;

            InternalNodeItemType & lookup_elt(std::size_t i) { return *(reinterpret_cast<InternalNodeItemType *>(&(elt_v_[i]))); }

            InternalNodeItemType const & lookup_elt(std::size_t i) const { return *(reinterpret_cast<InternalNodeItemType const *>(&(elt_v_[i]))); }

            FindNodeResult<GenericNodeType> find_child(Key const & key);

            /* insert node at position ix;  moving items starting in .elt_v[ix] one slot to the right */
            void insert_node(std::size_t ix, std::unique_ptr<GenericNodeType> child, bool debug_flag);

            /* remove node at position ix;  moving items starting .elt_v[ix+1] one slot to the left;
             * if target is a leaf node,  also remove from prev_leafnode/next_leafnode list
             */
            void remove_node(std::size_t ix, bool debug_flag);

            /* redistribute last n items from left-hand sibling lh to this internal node */
            void prepend_from_lh_sibling(InternalNode * lh, std::size_t n, bool debug_flag);

            /* redistribute first n items from right-hand sibling rh to this internal node */
            void append_from_rh_sibling(std::size_t n, InternalNode * rh);

            void append_rh_sibling(InternalNode * rh) { this->append_from_rh_sibling(rh->n_elt(), rh); }

            /* returns new node with upper half of original element vector (i.e. of this.elt_v[]);
             * original updated to retain lower half
             */
            std::unique_ptr<InternalNode> split_internal();

            void set_glb_key(Key key) { this->lookup_elt(0).set_key(key); }

            /* memory for InternalNode instances is always created using new[],
             * so required to use delete[] to deallocate
             */
            void operator delete (void * mem) noexcept { ::operator delete[](mem); }

            // ----- inherited from GenericNode -----

            virtual Key const & glb_key() const override { return this->lookup_elt(0).key(); }

            virtual std::size_t verify_helper(InternalNode const * parent,
                                              bool with_lub_flag,
                                              Key const & lub_key,
                                              LeafNodeType const * lh_leaf,
                                              LeafNodeType const * rh_leaf) const override;

            virtual void verify_glb_key(Key const & key) const override;

            /* find in subtree_arg the leftmost leaf node (i.e. leaf node with smallest key) */
            virtual FindNodeResult<LeafNodeType> find_min_leaf_node() override;
            /* find in subtree_arg the rightmost leaf node (i.e. leaf node with largest key) */
            virtual FindNodeResult<LeafNodeType> find_max_leaf_node() override;

        private:
            explicit InternalNode(std::size_t branching_factor);

        private:
#ifdef OBSOLETE
            /* total #of elements in this subtree */
            std::size_t size_ = 0;
#endif
            /* flexible array;  actual size will be .branching_factor().
             *
             * .elt_v[i] is created/destroyed as an InternalNodeItemType with non-trivial ctor/dtor.
             * we must declare member using POD placeholder to satisfy flexible array rules
             *
             * invariant:
             * - with branching factor b,  so range for .elt_v[] is 0 .. b-1:
             *   - .elt_v[j].child.ptr is null -> {.elt_v[j+1].child.ptr .. .elt_v[b-1].child.ptr} are also null
             */
            InternalNodeItemPlaceholderType elt_v_[];
        }; /*InternalNode*/

        template <typename Key, typename Value, typename Properties>
        InternalNode<Key, Value, Properties>::~InternalNode() {
            /* since we're using flexible array for .elt_v[],  need to manually run destructors */
            for (std::size_t i=0, n=this->branching_factor_; i<n; ++i) {
                this->lookup_elt(i).~InternalNodeItemType();
            }

            /* hygiene */
            BplusTreeUtil<Key, Value, Properties>::node_clear_size(this);
            this->n_elt_ = 0;
            this->branching_factor_ = 0;
        } /*dtor*/

        template <typename Key, typename Value, typename Properties>
        std::size_t
        InternalNode<Key, Value, Properties>::node_sizeof(std::size_t branching_factor) {
            return (sizeof(InternalNode)
                    + (branching_factor
                       * sizeof(InternalNodeItemType)));
        } /*node_sizeof*/

        template <typename Key, typename Value, typename Properties>
        std::unique_ptr<InternalNode<Key, Value, Properties>>
        InternalNode<Key, Value, Properties>::make_2(std::unique_ptr<GenericNodeType> child_1,
                                                     std::unique_ptr<GenericNodeType> child_2) {
            std::size_t branching_factor = child_1->branching_factor();

            std::size_t mem_z = node_sizeof(branching_factor);
            std::uint8_t * mem = new std::uint8_t[mem_z];

            assert(child_1->n_elt() > 0);
            assert(child_2->n_elt() > 0);

            std::unique_ptr<InternalNode> retval(new (mem) InternalNode(branching_factor));

            child_1->set_parent(retval.get());
            child_2->set_parent(retval.get());

            retval->assign_size(BplusTreeUtil<Key, Value, Properties>::get_node_size(child_1.get())
                                + BplusTreeUtil<Key, Value, Properties>::get_node_size(child_2.get()));
            retval->n_elt_ = 2;

            retval->lookup_elt(0) = std::move(InternalNodeItemType(std::move(child_1)));
            retval->lookup_elt(1) = std::move(InternalNodeItemType(std::move(child_2)));

            return retval;
        } /*make_2*/

        template <typename Key, typename Value, typename Properties>
        std::unique_ptr<InternalNode<Key, Value, Properties>>
        InternalNode<Key, Value, Properties>::annex(std::size_t mid_ix,
                                                    InternalNode * src)
        {
            std::size_t branching_factor = src->branching_factor();

            std::size_t mem_z = node_sizeof(branching_factor);
            std::uint8_t * mem = new std::uint8_t[mem_z];

            std::unique_ptr<InternalNode> new_node(new (mem) InternalNode(branching_factor));

            std::size_t hi_ix = src->n_elt();

            new_node->n_elt_ = hi_ix - mid_ix;

            std::size_t annex_z = 0;

            /* annexing upper-half of *src into new_node */
            for (std::size_t i = 0, n = hi_ix - mid_ix; i < n; ++i) {
                InternalNodeItemType & src_slot = src->lookup_elt(mid_ix + i);
                InternalNodeItemType & new_slot = new_node->lookup_elt(i);

                annex_z += BplusTreeUtil<Key, Value, Properties>::get_node_size(src_slot.child());

                new_slot = std::move(src->lookup_elt(mid_ix + i));
                new_slot.child()->set_parent(new_node.get());
            }

            new_node->assign_size(annex_z);

            /* ordinal_disabled: noop
             * ordinal_enabled:  bookkeeping for src.size (+ new_node.size, see above)
             */
            src->assign_size(BplusTreeUtil<Key, Value, Properties>::get_node_size(src) - annex_z);
            src->n_elt_ = mid_ix;

            return new_node;
        } /*annex*/

        template <typename Key, typename Value, typename Properties>
        std::size_t
        InternalNode<Key, Value, Properties>::find_lub_ix(Key const & key) const {
            if (key < this->lookup_elt(0).key())
                return 0;

            std::size_t lo = 0;
            std::size_t hi = this->n_elt_;

            while (lo + 1 < hi) {
                std::size_t mid = lo + (hi - lo) / 2;

                if (key < this->lookup_elt(mid).key())
                    hi = mid;
                else
                    lo = mid;
            }

            return hi;
        } /*find_lub_ix*/

        template <typename Key, typename Value, typename Properties>
        std::size_t
        InternalNode<Key, Value, Properties>::locate_child_by_address(GenericNodeType const * target_child) const {
            for (std::size_t ix = 0; ix < this->n_elt_; ++ix) {
                if (this->lookup_elt(ix).child() == target_child)
                    return ix;
            }

            return static_cast<std::size_t>(-1);
        } /*locate_child_by_address*/

        template <typename Key, typename Value, typename Properties>
        FindNodeResult<LeafNode<Key, Value, Properties>>
        InternalNode<Key, Value, Properties>::find_min_leaf_node() {
            FindNodeResult<GenericNodeType> findresult(0, this);

            while (findresult.node() && (findresult.node()->node_type() == NodeType::internal)) {
                std::size_t min_ix = 0;

                findresult = FindNodeResult<GenericNodeType>(min_ix,
                                                         (reinterpret_cast<InternalNodeType *>(findresult.node()))
                                                         ->lookup_elt(min_ix /*leftmost child*/).child());
            }

            /* findresult.node()->node_type() == NodeType::leaf (if non-null) */

            if (!findresult.node()) {
                assert(false);
                return FindNodeResult<LeafNodeType>();
            }

            assert(findresult.node()->node_type() == NodeType::leaf);

            return FindNodeResult<LeafNodeType>(findresult.ix(),
                                            reinterpret_cast<LeafNodeType *>(findresult.node()));
        } /*find_min_leaf_node*/

        template <typename Key, typename Value, typename Properties>
        FindNodeResult<LeafNode<Key, Value, Properties>>
        InternalNode<Key, Value, Properties>::find_max_leaf_node() {
            FindNodeResult<GenericNodeType> findresult(0, this);

            while (findresult.node() && (findresult.node()->node_type() == NodeType::internal)) {
                std::size_t max_ix = findresult.node()->n_elt() - 1;

                findresult = FindNodeResult<GenericNodeType>
                    (max_ix,
                     (reinterpret_cast<InternalNodeType *>(findresult.node()))
                     ->lookup_elt(max_ix /*rightmost child*/).child());
            }

            /* findresult.node()->node_type() == NodeType::leaf (if non-null) */

            if (!findresult.node()) {
                assert(false);
                return FindNodeResult<LeafNodeType>();
            }

            assert(findresult.node()->node_type() == NodeType::leaf);

            return FindNodeResult<LeafNodeType>(findresult.ix(),
                                            reinterpret_cast<LeafNodeType *>(findresult.node()));
        } /*find_max_leaf_node*/

        template <typename Key, typename Value, typename Properties>
        FindNodeResult<GenericNode<Key, Value, Properties>>
        InternalNode<Key, Value, Properties>::find_child(Key const & key) {
            std::size_t lub_ix = this->find_lub_ix(key);

            if (lub_ix > 0)
                --lub_ix;

            return FindNodeResult<GenericNodeType>(lub_ix, this->lookup_elt(lub_ix).child());
        } /*find_child*/

        template <typename Key, typename Value, typename Properties>
        void
        InternalNode<Key, Value, Properties>::insert_node(std::size_t ix, std::unique_ptr<GenericNodeType> child, bool debug_flag)
        {
            using xo::scope;
            using xo::tostr;
            using xo::xtag;

            scope log(XO_DEBUG(debug_flag),
                      xtag("self", this),
                      xtag("n_elt", this->n_elt()),
                      xtag("bf", this->branching_factor()),
                      xtag("ix", ix),
                      xtag("child", child.get()));

            if (this->n_elt_ >= this->branching_factor()) {
                assert(false);
                throw std::runtime_error(tostr("InternalNode::insert_node: node already full",
                                               xtag("node.n_elt", this->n_elt()),
                                               xtag("branching_factor", this->branching_factor())));
            }

            if (ix > this->n_elt_) {
                assert(false);
                throw std::runtime_error(tostr("InternalNode::insert_node: insert position out of range",
                                               xtag("ix", ix),
                                               xtag("node.n_elt", this->n_elt()),
                                               xtag("bf", this->branching_factor())));
            }

            std::size_t pos_ix = this->n_elt_;

            while (pos_ix > ix) {
                this->lookup_elt(pos_ix) = std::move(this->lookup_elt(pos_ix - 1));
                --pos_ix;
            }

            /* WARNING: don't update .size here
             *          in practice we use .insert_node() when introducing a single new key/value pair;
             *          when we use .insert_node() we split an existing node,
             *          and actually just want to increment .size.
             *
             *          We leave this to caller (e.g. BplusTree.internal_insert_aux())
             *          because in that context can see the upstream split
             */
            // this->size_ += child->n_elt();

            ++(this->n_elt_);
            child->set_parent(this);
            this->lookup_elt(ix) = InternalNodeItemType(std::move(child));
        } /*insert_node*/

        template <typename Key, typename Value, typename Properties>
        void
        InternalNode<Key, Value, Properties>::remove_node(std::size_t ix, bool debug_flag) {
            using xo::scope;
            using xo::tostr;
            using xo::xtag;

            scope log(XO_DEBUG(debug_flag),
                      xtag("self", this),
                      xtag("n_elt", this->n_elt()),
                      xtag("bf", this->branching_factor()),
                      xtag("ix", ix));

            if (ix >= this->n_elt_) {
                assert(false);
                throw std::runtime_error(tostr("InternalNode::remove_node: target position out of range",
                                               xtag("ix", ix),
                                               xtag("node.n_elt", this->n_elt()),
                                               xtag("bf", this->branching_factor())));
            }

            std::size_t pos_ix = ix;
            std::size_t end_ix = this->n_elt_ - 1;

            {
                InternalNodeItemType & target_item = this->lookup_elt(pos_ix);

                /* WARNING: don't update .size here
                 *          in practice we use .remove_node() when deleting a single new key/value pair;
                 *          when we use .remove_node() we merge existing nodes,
                 *          and actually just want to decrement .size.
                 *
                 *          We leave this to caller (e.g. BplusTree.internal_remove_aux())
                 *          because in that context can see the upstream merge
                 */
                //this->size_ -= target_item.child()->size();
                target_item.notify_remove();
            }

            while (pos_ix < end_ix) {
                //scope x1("loop", debug_flag);
                //x1(xtag("pos_ix", pos_ix));

                this->lookup_elt(pos_ix) = std::move(this->lookup_elt(pos_ix + 1));
                ++pos_ix;
            }

            --(this->n_elt_);
        } /*remove_node*/

        template <typename Key, typename Value, typename Properties>
        void
        InternalNode<Key, Value, Properties>::prepend_from_lh_sibling(InternalNode * lh, std::size_t n, bool debug_flag) {
            using xo::scope;
            using xo::xtag;

            scope log(XO_DEBUG(debug_flag),
                      xtag("@", this), xtag("n", n));

            if (this->n_elt() + n > this->branching_factor()) {
                assert(false);
                throw std::runtime_error(tostr("InternalNode.prepend_from_lh_sibling: expected combined #elt <= bf",
                                               xtag("self.n_elt", this->n_elt()),
                                               xtag("n", n),
                                               xtag("bf", this->branching_factor())));
            }

            std::size_t n_lh = lh->n_elt();
            std::size_t n_rh = this->n_elt();

            /* move elts in *this to the right n steps (starting from the end) */
            for (std::size_t ixp1 = this->n_elt(); ixp1 > 0; --ixp1) {
                std::size_t ix = ixp1 - 1;
                //x.log("move", xtag("ix", ix), xtag("ix+n", ix+n));
                this->lookup_elt(ix + n) = std::move(this->lookup_elt(ix));
            }

            std::size_t xfer_z = 0;

            /* xfer n elts from upper end of lh,  to lower end of *this */
            for (std::size_t ix = 0; ix < n; ++ix) {
                //x.log("fill", xtag("ix", ix), xtag("n_lh-n+ix", n_lh - n + ix));

                InternalNodeItemType & lh_sibling_item = lh->lookup_elt(n_lh - n + ix);

                xfer_z += BplusTreeUtil<Key, Value, Properties>::get_node_size(lh_sibling_item.child());

                this->lookup_elt(ix) = std::move(lh_sibling_item);
                /* + fixup parent pointer */
                this->lookup_elt(ix).child()->set_parent(this);
            }

            BplusTreeUtil<Key, Value, Properties>::node_add_size(this, xfer_z);
            BplusTreeUtil<Key, Value, Properties>::node_sub_size(lh, xfer_z);

            this->n_elt_ += n;
            lh->n_elt_ -= n;

            log && log(xtag("this.glb_key", this->glb_key()),
                       xtag("this[0].key", this->lookup_elt(0).key()));

            log.end_scope();
        } /*prepend_from_lh_sibling*/

        template <typename Key, typename Value, typename Properties>
        void
        InternalNode<Key, Value, Properties>::append_from_rh_sibling(std::size_t n, InternalNode * rh) {
            using xo::xtag;

            if (this->n_elt() + n > this->branching_factor()) {
                assert(false);
                throw std::runtime_error(tostr("InternalNode.append_from_rh_sibling: expected combined #elt <= bf",
                                               xtag("self.n_elt", this->n_elt()),
                                               xtag("n", n),
                                               xtag("bf", this->branching_factor())));
            }

            std::size_t n_lh = this->n_elt();
            std::size_t xfer_z = 0;

            for (std::size_t ix = 0; ix < n; ++ix) {
                InternalNodeItemType & rh_sibling_item = rh->lookup_elt(ix);

                xfer_z += BplusTreeUtil<Key, Value, Properties>::get_node_size(rh_sibling_item.child());
                this->lookup_elt(n_lh + ix) = std::move(rh_sibling_item);
                /* + fixup parent pointer */
                this->lookup_elt(n_lh + ix).child()->set_parent(this);
            }

            BplusTreeUtil<Key, Value, Properties>::node_add_size(this, xfer_z);
            this->n_elt_ += n;

            /* shuffle remaining members of rh sibling n items to the left */
            for (std::size_t ix = 0; ix < rh->n_elt() - n; ++ix) {
                rh->lookup_elt(ix) = std::move(rh->lookup_elt(ix + n));
            }

            BplusTreeUtil<Key, Value, Properties>::node_sub_size(rh, xfer_z);
            rh->n_elt_ -= n;
        } /*append_from_rh_sibling*/

        template <typename Key, typename Value, typename Properties>
        std::unique_ptr<InternalNode<Key, Value, Properties>>
        InternalNode<Key, Value, Properties>::split_internal() {
            std::size_t n_elt = this->n_elt_;
            std::size_t mid_ix = n_elt / 2;

            return InternalNode::annex(mid_ix, this);
        } /*split_internal*/

        template <typename Key, typename Value, typename Properties>
        std::size_t
        InternalNode<Key, Value, Properties>::verify_helper(InternalNode const * parent,
                                                            bool with_lub_flag,
                                                            Key const & lub_key,
                                                            LeafNodeType const * lh_leaf,
                                                            LeafNodeType const * rh_leaf) const
        {
            using xo::tostr;
            using xo::xtag;

            std::size_t retval = 0;

            /* verify immediate parent pointer is correct */
            if (this->parent() != parent) {
                throw std::runtime_error(tostr("InternalNode::verify_helper"
                                               ": expected parent pointer to refer to actual parent",
                                               xtag("stored_parent", this->parent()),
                                               xtag("actual_parent", parent)));
            }

            std::size_t n = this->n_elt_;

            /* verify all children have same NodeType (either all= internal or all= leaf) */
            NodeType target_child_node_type = NodeType::leaf;

            if (n > 0)
                target_child_node_type = this->lookup_elt(0).child()->node_type();

            LeafNodeType const * prev_lh_leaf = lh_leaf;

            for (std::size_t i=0; i < n; ++i) {
                /* check consistent node type */
                NodeType i_nodetype = this->lookup_elt(i).child()->node_type();

                if ((i > 0) && (i_nodetype != target_child_node_type)) {
                    throw std::runtime_error(tostr("InternalNode::verify_helper"
                                                   ": expected all children to share the same node type",
                                                   xtag("i", i),
                                                   xtag("elt[0].node_type", target_child_node_type),
                                                   xtag("elt[i].node_type", i_nodetype)));
                }

                /* nested verify on child subtrees */
                InternalNodeItemType const & i_elt = this->lookup_elt(i);

                LeafNodeType const * next_lh_leaf = ((i+1 < n)
                                                     ? this->lookup_elt(i+1).child()->find_min_leaf_node().node()
                                                     : rh_leaf);

                retval += i_elt.child()->verify_helper(this,
                                                       (i+1 < n) ? true : with_lub_flag,
                                                       (i+1 < n) ? this->lookup_elt(i+1).key() : lub_key,
                                                       prev_lh_leaf,
                                                       next_lh_leaf);

                prev_lh_leaf = i_elt.child()->find_max_leaf_node().node();
            }

            if (Properties::ordinal_tag_value() == tags::ordinal_enabled) {
                /* verify stored subtree size is consistent with children's */
                std::size_t sum_z = 0;

                for (std::size_t i=0, n=this->n_elt_; i < n; ++i) {
                    InternalNodeItemType const & elt = this->lookup_elt(i);

                    sum_z += BplusTreeUtil<Key, Value, Properties>::get_node_size(elt.child());
                }

                std::size_t self_z = BplusTreeUtil<Key, Value, Properties>::get_node_size(this);

                if (sum_z != self_z) {
                    throw std::runtime_error(tostr("InternalNode::verify_helper",
                                                   ": inconsistent subtree size",
                                                   xtag("node", this),
                                                   xtag("treez[stored]", self_z),
                                                   xtag("treez[computed]", sum_z)));
                }
            }

            /* verify stored glb_key is correct */
            for (std::size_t i=0, n=this->n_elt_; i < n; ++i) {
                InternalNodeItemType const & elt = this->lookup_elt(i);

                elt.child()->verify_glb_key(elt.key());
            }

            /* verify locally stored keys appear in sorted order */
            for (std::size_t i=1; i < n; ++i) {
                InternalNodeItemType const & prev = this->lookup_elt(i-1);
                InternalNodeItemType const & elt = this->lookup_elt(i);

                if (prev.key() < elt.key()) {
                    ;
                } else {
                    throw std::runtime_error(tostr("InternalNode::verify_helper"
                                                   ": expected local keys in strictly increasing order",
                                                   xtag("i", i),
                                                   xtag("key(i-1)", prev.key()),
                                                   xtag("key(i)", elt.key())));
                }
            }

            /* verify highest stored key before parent-supplied upper bound */
            if (with_lub_flag) {
                if (this->lookup_elt(n-1).key() < lub_key) {
                    ;
                } else {
                    throw std::runtime_error(tostr("InternalNode::verify_helper"
                                                   ": expected highest local key before parent-supplied lub key",
                                                   xtag("n", n),
                                                   xtag("key(n-1)", this->lookup_elt(n-1).key()),
                                                   xtag("lub_key", lub_key)));
                }
            }

            return retval;
        } /*verify_helper*/

        template <typename Key, typename Value, typename Properties>
        void
        InternalNode<Key, Value, Properties>::verify_glb_key(Key const & key) const {
            InternalNodeItemType const & elt = this->lookup_elt(0);

            elt.child()->verify_glb_key(key);
        } /*verify_glb_key*/

        template <typename Key, typename Value, typename Properties>
        InternalNode<Key, Value, Properties>::InternalNode(std::size_t branching_factor)
            : InternalNodeShim<Key, Value, Properties>{NodeType::internal, branching_factor}
        {
            /* must invoke ctor explicitly for each .elt_v[i].
             * compiler doesn't know extent of .elt_v[], since it's a flexible array
             */
            for (std::size_t i = 0; i < branching_factor; ++i) {
                /* using placement new to force ctor call inside flexible array */
                new (&(this->lookup_elt(i))) InternalNodeItemType();
            }
        } /*ctor*/

    } /*namespace tree*/
} /*namespace xo*/

/* end InternalNode.hpp */
