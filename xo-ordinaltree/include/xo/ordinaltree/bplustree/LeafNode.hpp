/* @file LeafNode.hpp */

#pragma once

#include "GenericNode.hpp"
#include "xo/indentlog/scope.hpp"
#include <cassert>

namespace xo {
    namespace tree {

        // ----- LeafNodeItem -----

        template <typename Key, typename Value, typename Properties>
        using LeafNodeItem = NodeItem<NodeType::leaf, Key, Value, Properties>;

        /* - define for symmetry with NodeItem<Key, Properties>
         * - LeafNodeItem doesn't contain a child pointer;
         *   it belongs inside a leaf mode,  which by definition doesn't have children
         */
        template <typename Key, typename Value, typename Properties>
        struct NodeItem<NodeType::leaf, Key, Value, Properties> {
        public:
            using ContentsType = std::pair<Key, Value>;

        public:
            NodeItem() = default;
            NodeItem(std::pair<Key, Value> kv) : kv_pair_{std::move(kv)} {}

            std::pair<Key, Value> const & kv_pair() const { return kv_pair_; }

            Key   const & key  () const { return kv_pair_.first;  }
            Value const & value() const { return kv_pair_.second; }

            void assign_value(Value x) { kv_pair_.second = std::move(x); }

        private:
            /* key+value pair */
            std::pair<Key, Value> kv_pair_;
        }; /*NodeItem*/

        /* struct with same size as LeafNodeItem<Key, Value, Properties>,  but POD + with no ctor/dtor */
        template <typename Key, typename Value, typename Properties>
        using LeafNodeItemPlaceholder = NodeItemPlaceholder<NodeType::leaf, Key, Value, Properties>;

        template <typename Key, typename Value, typename Properties, tags::ordinal_tag OrdinalTag = Properties::ordinal_tag_value()>
        struct LeafNodeShim : public GenericNode<Key, Value, Properties>
        {
            LeafNodeShim(NodeType ntype, std::size_t branching_factor) : GenericNode<Key, Value, Properties>(ntype, branching_factor) {}

            /* ordinal_enabled:   LeafNode will provide .size(): inherits+overrides GenericNodeBase.size() */
        };

        template <typename Key, typename Value, typename Properties>
        struct LeafNodeShim<Key, Value, Properties, tags::ordinal_disabled> : public GenericNode<Key, Value, Properties>
        {
            LeafNodeShim(NodeType ntype, std::size_t branching_factor) : GenericNode<Key, Value, Properties>(ntype, branching_factor) {}

            /* ordinal_disabled:   LeafNode provides LeafNode::size(),  but not used */

            virtual std::size_t size() const = 0;
        };

        // ----- LeafNode -----

        /* require:
         * - Properties.branching_factor()
         */
        template <typename Key, typename Value, typename Properties>
        struct LeafNode : public LeafNodeShim<Key, Value, Properties> {
        public:
            using GenericNodeType = GenericNode<Key, Value, Properties>;
            using LeafNodeType = LeafNode<Key, Value, Properties>;
            using LeafNodeItemType = LeafNodeItem<Key, Value, Properties>;
            using LeafNodeItemPlaceholderType = LeafNodeItemPlaceholder<Key, Value, Properties>;
            using InternalNodeType = InternalNode<Key, Value, Properties>;

            using ContentsType = typename LeafNodeItemType::ContentsType;

        public:
            virtual ~LeafNode();

            /* node size in bytes (increases with branching factor) */
            static std::size_t node_sizeof(std::size_t branching_factor);

            /* named ctor idiom.  enforce heap allocation + unique_ptr wrapper */
            static std::unique_ptr<LeafNode> make(std::pair<Key const, Value> kv_pair,
                                                  Properties const & properties);

            /* create+return new leaf node that contains all the items in *src from position [lo_ix, hi_ix),
             * after this operation size of *src is reduced by (hi_ix - lo_ix)
             */
            static std::unique_ptr<LeafNode> annex(std::size_t lo_ix,
                                                   std::size_t hi_ix,
                                                   LeafNode * src);

            LeafNode * prev_leafnode() const { return prev_leafnode_; }
            LeafNode * next_leafnode() const { return next_leafnode_; }

            /* .first: true if key in tree already
             * .second: index position of (strict) least upper bound in .elt_v[]
             *          if .n_elt, key has no upper bound in this node
             */
            std::pair<bool, std::size_t> find_lub_ix(Key const & key) const;

            LeafNodeItemType & lookup_elt(std::size_t i) { return *(reinterpret_cast<LeafNodeItemType *>(&(this->elt_v_[i]))); }

            LeafNodeItemType const & lookup_elt(std::size_t i) const { return *(reinterpret_cast<LeafNodeItemType const *>(&(this->elt_v_[i]))); }

            void assign_leaf_value(std::size_t elt_ix, Value value) {
                assert(elt_ix < this->n_elt_);

                this->lookup_elt(elt_ix).assign_value(std::move(value));
            } /*assign_leaf_value*/

            /* assign precdeing leaf node (= LH sibling if share same parent) */
            void assign_prev_leafnode(LeafNode * x) { prev_leafnode_ = x; }
            void assign_next_leafnode(LeafNode * x) { next_leafnode_ = x; }

            /* insert new leaf at position ix,  associating key -> value
             * (shuffle existing elements at ix, ix+1..  1 position to the right)
             */
            void insert_leaf_item(std::size_t ix,
                                  std::pair<Key const, Value> const & kv_pair,
                                  bool debug_flag);

            /* remove key,value pair at position ix */
            void remove_leaf(std::size_t ix, bool debug_flag);

            /* append n items from left-hand sibling,  as new left-most elements
             * require: combined #of items must be at most b = branching factor
             */
            void prepend_from_lh_sibling(LeafNode * lh, std::size_t n, bool debug_flag);

            /* apepnd n items from right-hand sibling,  as new right-most elements
             * require: combined #of items must be at most b = branching factor
             */
            void append_from_rh_sibling(std::size_t n, LeafNode * rh);

            void append_rh_sibling(LeafNode * rh) { this->append_from_rh_sibling(rh->n_elt(), rh); }

            /* returns new leaf with lower half of original element vector;
             * original updated to retain upper half
             */
            std::unique_ptr<LeafNode> split_leaf_lower();

            /* returns new leaf with upper half of original element vector;
             * original updated to retain lower half
             */
            std::unique_ptr<LeafNode> split_leaf_upper();

            /* memory for LeafNode instances is always created using new[],
             * so required to use delete[] to deallocate
             */
            void operator delete (void * mem) noexcept { ::operator delete[](mem); }

            // ----- Inherited from GenericNode -----

            virtual std::size_t size() const override { return this->n_elt(); }

            virtual Key const & glb_key() const override { return this->lookup_elt(0).key(); }

            virtual std::size_t verify_helper(InternalNodeType const * parent,
                                              bool with_lub_flag,
                                              Key const & lub_key,
                                              LeafNodeType const * lh_leaf,
                                              LeafNodeType const * rh_leaf) const override;
            virtual void verify_glb_key(Key const & key) const override;
            virtual FindNodeResult<LeafNodeType> find_min_leaf_node() override;
            virtual FindNodeResult<LeafNodeType> find_max_leaf_node() override;

            virtual void notify_remove() override;

        private:
            explicit LeafNode(std::size_t branching_factor);

            LeafNode(std::pair<Key const, Value> const & kv_pair,
                     std::size_t branching_factor);

            void assign_siblings(LeafNode * prev, LeafNode * next);

        private:
            /* previous LeafNode in key order,  immediately before (all the keys in) this node.
             * use to streamline inorder traversal.
             */
            LeafNode * prev_leafnode_ = nullptr;
            /* next LeafNode in key order,  immediately after (all the keys in) this node.
             * streamline inorder traversal.
             */
            LeafNode * next_leafnode_ = nullptr;
            /* flexible array;   actual capacity will be Properties.branching_factor();
             * but only members [0 .. n_elt-1] are defined.
             *
             * actual type of .elt_v[i] is LeafNodeItem<Key, Value, Properties>;
             * need to use POD LeafNodeItemPlaceholder<Key, Value, Properties> to satisfy flexible-array rules
             */
            LeafNodeItemPlaceholderType elt_v_[];
        }; /*LeafNode*/

        template <typename Key, typename Value, typename Properties>
        LeafNode<Key, Value, Properties>::~LeafNode() {
            /* since we're using flexible array for .elt_v[],  need to manually run destructors */
            for (std::size_t i=0, n=this->branching_factor_; i<n; ++i) {
                this->lookup_elt(i).~LeafNodeItemType();
            }

            /* hygiene */
            this->n_elt_ = 0;
            this->branching_factor_ = 0;
        } /*dtor*/

        template <typename Key, typename Value, typename Properties>
        std::size_t
        LeafNode<Key, Value, Properties>::node_sizeof(std::size_t branching_factor) {
            /* since we're using flexible array for .elt_v[],  need to manually account for it's allocated size */

            return (sizeof(LeafNode)
                    + (branching_factor
                       * sizeof(LeafNodeItem<Key, Value, Properties>)));
        } /*node_sizeof*/

        template <typename Key, typename Value, typename Properties>
        std::unique_ptr<LeafNode<Key, Value, Properties>>
        LeafNode<Key, Value, Properties>::make(std::pair<Key const, Value> kv_pair,
                                               Properties const & properties)
        {
            using xo::scope;
            using xo::xtag;

            std::size_t mem_z = node_sizeof(properties.branching_factor());
            /* storage for LeafNode,  including storage cost for flexible array LeafNode.elt_v[] */
            std::uint8_t * mem = new std::uint8_t[mem_z];

#ifdef NOT_IN_USE
            scope x("LeafNode.make");
            x.log(xtag("sizeof(LeafNode)", sizeof(LeafNode)),
                  xtag("bf", properties.branching_factor()),
                  xtag("mem_z", mem_z),
                  xtag("mem", (void *)mem));
#endif

            return std::unique_ptr<LeafNode>(new (mem) LeafNode(std::move(kv_pair),
                                                                properties.branching_factor()));
        } /*make*/

        template <typename Key, typename Value, typename Properties>
        std::unique_ptr<LeafNode<Key, Value, Properties>>
        LeafNode<Key, Value, Properties>::annex(std::size_t lo_ix,
                                                std::size_t hi_ix,
                                                LeafNode * src)
        {
            using xo::scope;
            using xo::xtag;

            std::size_t branching_factor = src->branching_factor();

            assert(hi_ix >= lo_ix);
            assert(hi_ix - lo_ix <= branching_factor);

            std::size_t mem_z = node_sizeof(branching_factor);
            std::uint8_t * mem = new std::uint8_t[mem_z];

#ifdef NOT_IN_USE
            scope x("LeafNode.annex");
            x.log(xtag("sizeof(LeafNode)", sizeof(LeafNode)),
                  xtag("bf", branching_factor),
                  xtag("mem_z", mem_z),
                  xtag("mem", (void *)mem));
#endif

            std::unique_ptr<LeafNode> new_node(new (mem) LeafNode(branching_factor));

            std::size_t old_n = src->n_elt();

            new_node->n_elt_ = hi_ix - lo_ix;

            std::size_t n_annex = hi_ix - lo_ix;

            /* annexing from *src into new_node */
            for (std::size_t i = 0; i < n_annex; ++i) {
                LeafNodeItemType & new_slot = new_node->lookup_elt(i);

                new_slot = std::move(src->lookup_elt(lo_ix + i));
            }

            /* shuffle over any remaining items in *src starting from hi_ix */
            for (std::size_t i = lo_ix; i + n_annex < old_n; ++i) {
                LeafNodeItemType & slot = src->lookup_elt(i);

                slot = std::move(src->lookup_elt(i + n_annex));
            }

            src->n_elt_ = old_n - n_annex;

            if (lo_ix == 0) {
                /* new node builds by taking leftmost elements from src
                 *  -> new node becomes src's predecessor
                 */
                new_node->assign_siblings(src->prev_leafnode(), src);
            } else {
                /* new node builds by taking rightmost elements from src
                 *  -> new node becomes src's successor
                 */
                new_node->assign_siblings(src, src->next_leafnode());
            }

            return new_node;
        } /*annex*/

        template <typename Key, typename Value, typename Properties>
        std::pair<bool, std::size_t>
        LeafNode<Key, Value, Properties>::find_lub_ix(Key const & key) const {
            if (key < this->lookup_elt(0).key())
                return std::make_pair(false, 0);

            /* promise: return value >= 0 */

            /* .elt_v[0 .. n_elt-1] are maintained in sorted key order */
            std::size_t lo = 0;
            std::size_t hi = this->n_elt_;

            while (lo + 1 < hi) {
                /* desired child item will be in range [lo, hi) */

                std::size_t mid = lo + (hi - lo) / 2;

                if (key < this->lookup_elt(mid).key())
                    hi = mid;
                else
                    lo = mid;
            }

            /* invariant:
             * - lo is a valid index: elt_v[lo].kv_pair reflects outcome of most recent call to BplusTree.insert()
             * - .elt_v[lo].key <= key
             * - if hi<.n_elt,  then key < .elt_v[hi].key
             */
            bool presence_flag = (key == this->lookup_elt(lo).key());

            return std::make_pair(presence_flag, hi);
        } /*find_lub_ix*/

        template <typename Key, typename Value, typename Properties>
        void
        LeafNode<Key, Value, Properties>::insert_leaf_item(std::size_t ix,
                                                           std::pair<Key const, Value> const & kv_pair,
                                                           bool debug_flag) {
            using xo::scope;
            using xo::xtag;

            scope log(XO_DEBUG(debug_flag),
                      xtag("self", this),
                      xtag("n_elt", this->n_elt()),
                      xtag("bf", this->branching_factor()),
                      xtag("ix", ix),
                      xtag("key", kv_pair.first),
                      xtag("value", kv_pair.second));

            if (this->n_elt_ >= this->branching_factor()) {
                assert(false);
                throw std::runtime_error(tostr("LeafNode::insert_leaf: leaf already full",
                                               xtag("leaf.n_elt", this->n_elt()),
                                               xtag("branching_factor", this->branching_factor())));
            }

            std::size_t pos_ix = this->n_elt_;

            while (pos_ix > ix) {
                //scope x1("loop");
                //x1.log(xtag("pos_ix", pos_ix));

                this->lookup_elt(pos_ix) = std::move(this->lookup_elt(pos_ix - 1));
                --pos_ix;
            }

            ++(this->n_elt_);
            this->lookup_elt(ix) = LeafNodeItemType(kv_pair);

            log.end_scope();
        } /*insert_leaf*/

        template <typename Key, typename Value, typename Properties>
        void
        LeafNode<Key, Value, Properties>::remove_leaf(std::size_t ix, bool debug_flag) {
            using xo::scope;
            using xo::xtag;

            scope log(XO_DEBUG(debug_flag),
                      xtag("self", this),
                      xtag("n_elt", this->n_elt()),
                      xtag("bf", this->branching_factor()),
                      xtag("ix", ix));

            if (this->n_elt_ == 0) {
                throw std::runtime_error(tostr("LeafNode::remove_leaf: leaf already empty",
                                               xtag("leaf.n_elt", this->n_elt()),
                                               xtag("branching_factor", this->branching_factor())));
            }

            /* TODO: removal action for position pos_ix (maintain reductions) */

            std::size_t pos_ix = ix;
            std::size_t end_ix = this->n_elt_ - 1;

            while (pos_ix < end_ix) {
                //scope x1("loop");
                //x1.log(xtag("pos_ix", pos_ix));

                this->lookup_elt(pos_ix) = std::move(this->lookup_elt(pos_ix + 1));
                ++pos_ix;
            }

            --(this->n_elt_);
        } /*remove_leaf*/

        template <typename Key, typename Value, typename Properties>
        void
        LeafNode<Key, Value, Properties>::prepend_from_lh_sibling(LeafNode<Key, Value, Properties> * lh, std::size_t n, bool debug_flag) {
            using xo::scope;
            using xo::xtag;

            scope log(XO_DEBUG(debug_flag),
                      xtag("n", n));

            if (this->n_elt() + n > this->branching_factor()) {
                assert(false);
                throw std::runtime_error(tostr("LeafNode.prepend_from_lh_sibling: expected combined #elt <= bf",
                                               xtag("self.n_elt", this->n_elt()),
                                               xtag("n", n),
                                               xtag("bf", this->branching_factor())));
            }

            std::size_t n_lh = lh->n_elt();
            std::size_t n_rh = this->n_elt();

            /* move elts in *this to the right n steps */
            for (std::size_t ixp1 = this->n_elt(); ixp1 > 0; --ixp1) {
                std::size_t ix = ixp1 - 1;
                this->lookup_elt(ix + n) = std::move(this->lookup_elt(ix));
            }

            /* xfer n elts from upper end of lh,  to lower end of *this */
            for (std::size_t ix = 0; ix < n; ++ix) {
                this->lookup_elt(ix) = lh->lookup_elt(n_lh - n + ix);
            }

            this->n_elt_ += n;
            lh->n_elt_ -= n;

            /* note:  since we didn't create/destroy any LeafNodes,
             *        .prev_leafnode / .next_leafnode pointers are unchanged
             */

            log.end_scope();
        } /*prepend_from_lh_sibling*/

        template <typename Key, typename Value, typename Properties>
        void
        LeafNode<Key, Value, Properties>::append_from_rh_sibling(std::size_t n, LeafNode<Key, Value, Properties> * rh) {
            using xo::xtag;

            if (this->n_elt() + n > this->branching_factor()) {
                assert(false);
                throw std::runtime_error(tostr("LeafNode.append_from_rh_sibling: expected combined #elt <= bf",
                                               xtag("self.n_elt", this->n_elt()),
                                               xtag("n", n),
                                               xtag("bf", this->branching_factor())));
            }

            std::size_t n_lh = this->n_elt();

            for (std::size_t ix = 0; ix < n; ++ix) {
                this->lookup_elt(n_lh + ix) = std::move(rh->lookup_elt(ix));
                /* note: leaf items are key,value pairs;
                 *       no parent pointers to fixup (cf InternalNode.append_from_rh_sibling)
                 */
            }

            this->n_elt_ += n;

            /* shuffle remaining members of rh sibling n items to the left */
            for (std::size_t ix = 0; ix < rh->n_elt() - n; ++ix) {
                rh->lookup_elt(ix) = std::move(rh->lookup_elt(ix + n));
            }

            rh->n_elt_ -= n;

            /* note:  since we didn't create/destroy any LeafNodes,
             *        .prev_leafnode / .next_leafnode pointers are unchanged
             */

        } /*append_from_rh_sibling*/

        template <typename Key, typename Value, typename Properties>
        std::unique_ptr<LeafNode<Key, Value, Properties>>
        LeafNode<Key, Value, Properties>::split_leaf_lower() {
            std::size_t n_elt = this->n_elt_;
            std::size_t mid_ix = n_elt / 2;

            return LeafNode::annex(0, mid_ix, this);
        } /*split_leaf_lower*/

        template <typename Key, typename Value, typename Properties>
        std::unique_ptr<LeafNode<Key, Value, Properties>>
        LeafNode<Key, Value, Properties>::split_leaf_upper() {
            std::size_t n_elt = this->n_elt_;
            std::size_t mid_ix = n_elt / 2;

            return LeafNode<Key, Value, Properties>::annex(mid_ix, n_elt, this);
        } /*split_leaf_upper*/

        template <typename Key, typename Value, typename Properties>
        std::size_t
        LeafNode<Key, Value, Properties>::verify_helper(InternalNodeType const * parent,
                                                        bool with_lub_flag,
                                                        Key const & lub_key,
                                                        LeafNodeType const * lh_leaf,
                                                        LeafNodeType const * rh_leaf) const {
            using xo::xtag;

            /* verify immediate parent pointer is correct */
            if (this->parent() != parent) {
                throw std::runtime_error(tostr("LeafNode::verify_helper"
                                               ": expected parent pointer to refer to actual parent",
                                               xtag("stored_parent", this->parent()),
                                               xtag("actual_parent", parent)));
            }

            /* verify locally stored keys appear in sorted order */
            std::size_t n = this->n_elt_;
            for (std::size_t i=1; i < n; ++i) {
                LeafNodeItemType const & prev = this->lookup_elt(i-1);
                LeafNodeItemType const & elt = this->lookup_elt(i);

                if (prev.key() < elt.key()) {
                    ;
                } else {
                    throw std::runtime_error(tostr("LeafNode::verify_helper"
                                                   ": expected local keys in strictly increasing order",
                                                   xtag("i", i),
                                                   xtag("key(i-1)", prev.key()),
                                                   xtag("key(i)", elt.key())));
                }
            }

            if (with_lub_flag) {
                if (this->lookup_elt(n-1).key() < lub_key) {
                    ;
                } else {
                    throw std::runtime_error(tostr("LeafNode::verify_helper"
                                                   ": expected last local key before parent-supplied lub key",
                                                   xtag("n", n),
                                                   xtag("key(n-1)", this->lookup_elt(n-1).key()),
                                                   xtag("lub_key", lub_key)));
                }
            }

            /* verify next/prev leafnode pointers are consistent */
            if ((lh_leaf && (lh_leaf->next_leafnode() != this))
                || (this->prev_leafnode() != lh_leaf))
            {
                throw std::runtime_error(tostr("LeafNode::verify_helper"
                                               ": inconsistent prev/next leaf pointers",
                                               xtag("parent", parent),
                                               xtag("lh_leaf", lh_leaf),
                                               xtag("lh_leaf.next", lh_leaf ? lh_leaf->next_leafnode() : nullptr),
                                               xtag("self", this),
                                               xtag("self.prev", this->prev_leafnode())));
            }

            if ((this->next_leafnode() != rh_leaf)
                || (rh_leaf && (rh_leaf->prev_leafnode() != this)))
            {
                throw std::runtime_error(tostr("LeafNode::verify_helper"
                                               ": inconsistent prev/next leaf pointers",
                                               xtag("parent", parent),
                                               xtag("self", this),
                                               xtag("self.next", this->next_leafnode()),
                                               xtag("rh_leaf", rh_leaf),
                                               xtag("rh_leaf.prev", rh_leaf ? rh_leaf->prev_leafnode() : nullptr)));
            }

            return this->n_elt();
        } /*verify_helper*/

        template <typename Key, typename Value, typename Properties>
        void
        LeafNode<Key, Value, Properties>::verify_glb_key(Key const & key) const {
            using xo::xtag;

            LeafNodeItemType const & elt = this->lookup_elt(0);

            if (elt.key() != key) {
                throw std::runtime_error(tostr("LeafNode::verify_glb_key"
                                               ": expected stored greatest-lower-bound key to match leftmost leaf's key",
                                               xtag("@", this),
                                               xtag("reported_key", key),
                                               xtag("actual_key", elt.key())));
            }
        } /*verify_glb_key*/

        template <typename Key, typename Value, typename Properties>
        FindNodeResult<LeafNode<Key, Value, Properties>>
        LeafNode<Key, Value, Properties>::find_min_leaf_node() {
            return FindNodeResult<LeafNode<Key, Value, Properties>>(0, this);
        } /*find_min_leaf_node*/

        template <typename Key, typename Value, typename Properties>
        FindNodeResult<LeafNode<Key, Value, Properties>>
        LeafNode<Key, Value, Properties>::find_max_leaf_node() {
            return FindNodeResult<LeafNode<Key, Value, Properties>>(0, this);
        } /*c_find_max_leaf_node*/

        template <typename Key, typename Value, typename Properties>
        void
        LeafNode<Key, Value, Properties>::notify_remove() {
            if (this->prev_leafnode_)
                this->prev_leafnode_->assign_next_leafnode(this->next_leafnode_);
            if (this->next_leafnode_)
                this->next_leafnode_->assign_prev_leafnode(this->prev_leafnode_);
        } /*notify_remove*/

        template <typename Key, typename Value, typename Properties>
        LeafNode<Key, Value, Properties>::LeafNode(std::size_t branching_factor)
            : LeafNodeShim<Key, Value, Properties>(NodeType::leaf, branching_factor)
        {
            /* must call ctor explicitly for each element.
             * compiler can't do this for us, b/c it doesn't know size of flexible array
             */
            for (std::size_t i = 0, n = branching_factor; i < n; ++i) {
                new (&(this->lookup_elt(i))) LeafNodeItemType();
            }
        }

        template <typename Key, typename Value, typename Properties>
        LeafNode<Key, Value, Properties>::LeafNode(std::pair<Key const, Value> const & kv_pair,
                                                   std::size_t branching_factor)
            : LeafNodeShim<Key, Value, Properties>(NodeType::leaf, branching_factor)
        {
            using xo::scope;
            using xo::xtag;

#ifdef NOT_USING_DEBUG
            scope x("LeafNode.ctor");
#endif

            this->n_elt_ = 1;
            /* since .elt_v[] is a flexible array,  need to invoke constructors explicitly
             * (compiler doesn't know how many elements there are -> can't do it for us
             */

#ifdef NOT_USING_DEBUG
            x.log(xtag("elt[0]", &(this->lookup_elt(0))));
#endif

            new (&(this->lookup_elt(0))) LeafNodeItemType(kv_pair);

            for (std::size_t i = 1, n = branching_factor; i < n; ++i) {
#ifdef NOT_USING_DEBUG
                x.log(xtag("i", i),
                      xtag("elt[i]", &(this->lookup_elt(i))));
#endif

                /* using placement-new to invoke ctor explicitly */
                new (&(this->lookup_elt(i))) LeafNodeItemType();
            }
        } /*ctor*/

        template <typename Key, typename Value, typename Properties>
        void
        LeafNode<Key, Value, Properties>::assign_siblings(LeafNode * p, LeafNode * n) {
            if (p)
                p->assign_next_leafnode(this);
            this->prev_leafnode_ = p;
            this->next_leafnode_ = n;
            if (n)
                n->assign_prev_leafnode(this);
        } /*assign_siblings*/

    } /*namespace tree*/
} /*namespace xo*/


/* end LeafNode.hpp */
