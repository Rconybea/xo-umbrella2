/** @file RedBlackTree.hpp
 *
 *  @author Roland Conybaeare, Nov 2025
 **/

/* provides red-black tree with order statistics.
 */

#pragma once

#include "rbtree/NullReduce.hpp"
#include "rbtree/RbTreeUtil.hpp"
#include "rbtree/RbTreeLhs.hpp"
#include "rbtree/Iterator.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/pad.hpp"
#include "xo/indentlog/print/quoted.hpp"
#include <concepts>
#include <iterator>
#include <array>
#include <cmath>
#include <cassert>
#include <stdexcept>

namespace xo {
    namespace tree {

        /** @class RedBlackTree
         *  @brief red-black tree with order statistics
         *
         *  Lazily balanced. Longest path to a leaf is at most 2x the length of shortest path.
         *
         *  Maintains order statistics. Accumulates some associative relation on key,value pairs.
         *
         *  Can obtain iterator to k'th element of a tree with n nodes in log(n) time.
         *  Allows behaving as a weak random-access iterator with log(n) cost per query
         *
         *  Supported Features:
         *  1.  insert
         *  2.  erase
         *  3.  find
         *  3a. find by reduced value (provided reduction is monotonic)
         *  4.  forward + backward iteration
         *  5.  inorder visitor
         *  6.  operator[] (including no-assignment const version)
         *  7.  runtime verify class invariants
         *
         *  Missing Features:
         *  1. efficient iterator arithmetic
         *  2. pretty printing
         *  3. reflection support
         *  4. custom allocation support [works for std::allocator, probably. WIP for GC]
         *  5. custom key compare
         *  6. garbage collector integration
         *  7. std library integration
         **/
        template <typename Key,
                  typename Value,
                  typename Reduce,
                  typename Allocator>
        class RedBlackTree : public xo::gc::gc_allocator_traits<Allocator>::object_interface_type {
            static_assert(ordered_key<Key>);
            static_assert(valid_rbtree_reduce_functor<Reduce, Value>);

        public:
            using key_type = Key;
            using mapped_type = Value;
            using value_type = std::pair<Key const, Value>;

            // using key_compare = Compare // not yet
            using allocator_type = Allocator;
            using allocator_traits = xo::gc::gc_allocator_traits<Allocator>;

            /** WARNING:
             *  - GcObjectInterface is typed + assoc type is pair<Key const, Value>.
             **/
            using GcObjectInterface = allocator_traits::object_interface_type;
            using ReducedValue = typename Reduce::value_type;
            using RbUtil = detail::RbTreeUtil<Key, Value, Reduce, GcObjectInterface>;
            using RbNode = detail::Node<Key, Value, Reduce, GcObjectInterface>;
            using RbTreeLhs = detail::RedBlackTreeLhs<RedBlackTree>;
            using RbTreeConstLhs = detail::RedBlackTreeConstLhs<RedBlackTree>;

            using node_type = RbNode;
            using node_allocator_type = allocator_traits::template rebind_alloc<node_type>;
            using node_allocator_traits = xo::gc::gc_allocator_traits<node_allocator_type>;

            /* for make() factory method */
            using tree_allocator_type = allocator_traits::template rebind_alloc<RedBlackTree>;
            using tree_allocator_traits = xo::gc::gc_allocator_traits<tree_allocator_type>;

            using Direction = detail::Direction;
            using size_type = std::size_t;
            using difference_type = std::ptrdiff_t;
            using iterator = detail::Iterator<Key, Value, Reduce, GcObjectInterface>;
            using const_iterator = detail::ConstIterator<Key, Value, Reduce, GcObjectInterface>;

            using Reflect = xo::reflect::Reflect;
            using TaggedPtr = xo::reflect::TaggedPtr;

        public:
            explicit RedBlackTree(const allocator_type & alloc = allocator_type{},
                                  bool debug_flag = false) :
                                      node_alloc_{alloc},
                                      debug_flag_{debug_flag} {}
#ifdef NOT_YET // need copy_from
            RedBlackTree(const RedBlackTree & other) :
                node_alloc_{node_allocator_traits::select_on_container_copy_construction(other.node_alloc_)}
            //, compare_{other.compare_}
            {
                copy_from(other);
            }

            // similar: copy constructor with explicit alloc
            RedBlackTree(const RedBlackTree & other, const allocator_type & alloc) :
                node_alloc_{alloc}
            {
                copy_from(other);
            }

            // move ctor
            RedBlackTree(RedBlackTree && other) noexcept :
                node_alloc_{std::move(other.node_alloc_)},
                root_{other.root_}, size_{other.size}
            // , compare_{other.compare_)}
            {}
#endif

            static RedBlackTree * make(allocator_type kvpair_alloc = allocator_type{},
                                       bool debug_flag = false)
            {
                tree_allocator_type tree_alloc(kvpair_alloc);

                RedBlackTree * tree = tree_allocator_traits::allocate(tree_alloc, 1);

                try {
                    // placement new
                    tree_allocator_traits::construct(tree_alloc, tree, kvpair_alloc, debug_flag);
                    return tree;
                } catch(...) {
                    tree_allocator_traits::deallocate(tree_alloc, tree, 1);
                }

                return nullptr;
            }

            bool empty() const { return size_ == 0; }
            size_type size() const { return size_; }
            size_type max_size() const { return std::numeric_limits<difference_type>::max(); }
            Reduce const & reduce_fn() const { return reduce_fn_; }

            /* forward const iterators (canonical names) */

            /* iterator "one before beginning" */
            const_iterator cprebegin() const {
                return const_iterator::prebegin_aux(RbUtil::find_leftmost(this->root_));
            } /*cprebegin*/

            const_iterator cbegin() const {
                return const_iterator::begin_aux(RbUtil::find_leftmost(this->root_));
            } /*begin*/

            const_iterator cend() const {
                return const_iterator::end_aux(RbUtil::find_rightmost(this->root_));
            } /*end*/

            /* forward const iterators (overloaded names) */

            const_iterator prebegin() const { return this->cprebegin(); }
            const_iterator begin() const { return this->cbegin(); }
            const_iterator end() const { return this->cend(); }

            /* forward non-const iterators */

            iterator prebegin() {
                return iterator::prebegin_aux(RbUtil::find_leftmost(this->root_));
            } /*prebegin*/

            iterator begin() {
                return iterator::begin_aux(RbUtil::find_leftmost(this->root_));
            } /*begin*/

            iterator end() {
                return iterator::end_aux(RbUtil::find_rightmost(this->root_));
            } /*end*/

            /* reverse const iterators (canonical names) */

            /* reverse-iterator, "one after end" */
            const_iterator crprebegin() const {
                return const_iterator::rprebegin_aux(RbUtil::find_rightmost(this->root_));
            } /*crprebegin*/

            const_iterator crbegin() const {
                return const_iterator::rbegin_aux(RbUtil::find_rightmost(this->root_));
            } /*crbegin*/

            const_iterator crend() const {
                return const_iterator::rend_aux(RbUtil::find_leftmost(this->root_));
            } /*crend*/

            /* reverse const iterators (overloaded names) */

            const_iterator rprebegin() const { return this->crprebegin(); }
            const_iterator rbegin() const { return this->crbegin(); }
            const_iterator rend() const { return this->crend(); }

            /* reverse non-const iterators */

            iterator rprebegin() {
                return iterator::rprebegin_aux(RbUtil::find_rightmost(this->root_));
            } /*rprebegin*/

            iterator rbegin() {
                return iterator::rbegin_aux(RbUtil::find_rightmost(this->root_));
            }  /*rbegin*/

            iterator rend() {
                return iterator::rend_aux(RbUtil::find_leftmost(this->root_));
            } /*rend*/

            /* require:
             * - .size() > 0
             */
            Key const & min_key() const { return this->cbegin().first; }
            /* require:
             * - .size() > 0
             */
            Key const & max_key() const { const_iterator ix = this->cend(); --ix; return ix->first; }

            /* visit tree contents in increasing key order
             *
             * Require:
             * - Fn(std::pair<Key, Value> const &)
             */
            template<typename Fn>
            void visit_inorder(Fn && fn) {
                auto visitor_fn = [&fn](RbNode const * x, uint32_t /*d*/) { fn(x->contents()); };

                RbUtil::inorder_node_visitor(this->root_,
                                             0 /*depth -- will be ignored*/,
                                             visitor_fn);
            } /*visit_inorder*/

            /* if i in [0 .. .size], return iterator referring to ith inorder node in tree
             * otherwise return this->end()
             */
            const_iterator find_ith(uint32_t i) const {
                RbNode * node = RbUtil::find_ith(this->root_, i);

                if(node) {
                    return const_iterator(detail::ID_Forward, detail::IL_Regular, node);
                } else {
                    return this->end();
                }
            } /*find_ith*/

            iterator find_ith(uint32_t i) {
                RbNode * node = RbUtil::find_ith(this->root_, i);

                if(node) {
                    return iterator(detail::IL_Regular, node);
                } else {
                    return this->end();
                }
            } /*find_ith*/

            /* find node with key equal to x in this tree.
             * on success,  return iterator ix with ix->first = x.
             * on failure,  return this->end()
             */
            const_iterator find(Key const & x) const {
                RbNode * node = RbUtil::find(this->root_, x);

                if(node) {
                    return const_iterator(detail::ID_Forward, detail::IL_Regular, node);
                } else {
                    return this->end();
                }
            } /*find*/

            iterator find(Key const & x) {
                RbNode * node = RbUtil::find(this->root_, x);

                if (node) {
                    return const_iterator(detail::ID_Forward, detail::IL_Regular, node);
                } else {
                    return this->end();
                }
            } /*find*/

            /* find node in tree with largest key k such that:
             *   k <= x,  if  is_closed
             *   k <  x,  if !is_closed
             *
             * return iterator to that node.
             *
             * If no such node exists,  return the same value as this->cprebegin();
             *
             * This satisfies continuity property:
             *   if:   ix   = find_glb(k,  is_closed),
             *   then: ix+1 = find_lub(k, !is_closed)
             *
             * even when ix.is_dereferenceable() is false
             */
            const_iterator find_glb(Key const & k, bool is_closed) const {
                RbNode * node = RbUtil::find_glb(this->root_, k, is_closed);

                if (node) {
                    return const_iterator(detail::ID_Forward,
                                          detail::IL_Regular,
                                          node);
                } else {
                    return this->cprebegin();
                }
            } /*find_glb*/

            const_iterator find_lub(Key const & k, bool is_closed) const {
                const_iterator ix = this->find_glb(k, !is_closed);
                return ++ix;
            } /*find_lub*/

            /* RbTreeConstLhs provides rvalue-substitute for lookup-only in const RedBlackTree
             * instances
             */
            RbTreeConstLhs operator[](Key const & k) const
                {
                    //scope log(XO_DEBUG(true), xtag("variant", "readonly"), xtag("key", k));

                    RbNode const * node = RbUtil::find(this->root_, k);

                    return RbTreeConstLhs(this, node);
                } /*operator[]*/

            /* RbTreeLhs defers assignment,  so that rbtree can update values of
             * Node::reduce along path from root to Node n with n.key = k
             *
             *
             * Note:
             * 1. return value remains valid across subsequent inserts and assignments,
             *    so this is legal:
             *      RbTree rbtree = ...;
             *      auto v = rbtree[key1];
             *
             *      rbtree[key2] = ...;
             *      rbtree.insert(key3, value3);
             *
             *      v = ...;
             *
             * 2. return value is not valid across removes,  even of distinct keys,
             *    so this is ILLEGAL:
             *      RbTree rbtree = ...;
             *      auto v = rbtree[key1];
             *
             *      assert(key1 != key2);
             *
             *      rbtree.remove(key2);
             *
             *      v = ...;   // undefined behavior,
             *                 // v.node contents may have been copied and v.node deleted
             */
            RbTreeLhs operator[](Key const & k) {
                //scope log(XO_DEBUG(true), tag("variant", "autoinsert"), tag("key", k));

                std::pair<bool, RbNode *> insert_result
                    = RbUtil::template insert_aux<node_allocator_type>(this->node_alloc_,
                                                                       value_type(k, Value() /*used iff creating new node*/),
                                                                       false /*allow_replace_flag*/,
                                                                       this->reduce_fn_,
                                                                       this->debug_flag_,
                                                                       &(this->root_));

                return RbTreeLhs(this, insert_result.second, k);
            } /*operator[]*/

            /* compute value of reduce applied to the set K of all keys k[j] in subtree
             * N with:
             * - k[j] <= lub_key  if is_closed = true
             * - k[j] <  lub_key  if is_closed = false
             * return reduce_fn.nil() if K is empty
             */
            ReducedValue reduce_lub(Key const &lub_key, bool is_closed) const {
                return RbUtil::reduce_lub(lub_key,
                                          this->reduce_fn_,
                                          is_closed,
                                          this->root_);
            } /*reduce_lub*/

            /* Provided Reduce computes sum,  and we call this rbtree f
             * with keys k[i] and values v[i]:
             *
             * returns iterator pointing to i'th key-value pair {k[i],v[i]} in this tree,
             * with reduced value r(i) (i.e. RbNode::reduced1);
             * where r(i) is the result of reducing all values v[j] with j<=i
             *
             * editor bait: invert_integral
             */
            const_iterator cfind_sum_glb(ReducedValue const & y) const {
                using xo::tostr;
                using xo::xtag;

                //char const * c_self = "RedBlackTree::find_sum_glb";

                RbNode * N = RbUtil::find_sum_glb(this->reduce_fn_,
                                                  this->root_,
                                                  y);

                if(!N) {
                    /* for no-lower-bound edge cases,  return iterator ix
                     * pointing to 'before the beginning' of this tree.
                     *
                     * will have
                     *   ix.is_deferenceable() == false
                     *   (bool)ix == false
                     */
                    return const_iterator(detail::ID_Forward,
                                          detail::IL_BeforeBegin,
                                          RbUtil::find_leftmost(this->root_));
                }

                return const_iterator(detail::ID_Forward,
                                      detail::IL_Regular,
                                      N);
            } /*cfind_sum_glb*/

            const_iterator find_sum_glb(ReducedValue const & y) const {
                return this->cfind_sum_glb(y);
            } /*find_sum_glb*/

            /* non-const version of .cfind_sum_glb() */
            iterator find_sum_glb(ReducedValue const & y) {
                const_iterator ix = this->cfind_sum_glb(y);

                return iterator(ix.location(),
                                const_cast<RbNode *>(ix.node()));
            } /*find_sum_glb*/

            void clear() {
                if constexpr (node_allocator_traits::has_trivial_deallocate_v) {
                    // nothing to do since trivial deallocator
                } else {
                    // visitor to delete all Nodes
                    auto visitor_fn = [this](RbNode const * x, uint32_t depth) {
                        (void)depth;

                        /* RbUtil.postorder_node_visitor() isn't expecting us to
                         * alter node,  but will not examine it after it's deleted
                         */
                        RbNode * xx = const_cast<RbNode *>(x);

                        node_allocator_traits::deallocate(node_alloc_, xx, 1);
                    };

                    RbUtil::postorder_node_visitor(this->root_,
                                                   0 /*depth -- ignored by lambda*/,
                                                   visitor_fn);

                };

                this->size_ = 0;
                this->root_ = nullptr;
            } /*clear*/

            std::pair<iterator, bool>
            insert(std::pair<Key const, Value> const & kv_pair) {
                RbNode * adj_root = this->root_;

                std::pair<bool, RbNode *> insert_result
                    = RbUtil::insert_aux(kv_pair,
                                         true /*allow_replace_flag*/,
                                         this->reduce_fn_,
                                         &adj_root);

                if (insert_result.first) {
                    ++(this->size_);

                    if (adj_root != root_) {
                        allocator_type alloc = node_alloc_;

                        this->_gc_assign_member(&(this->root_), adj_root, alloc);
                    }
                } else {
                    assert(adj_root == root_);
                }

                return (std::pair<iterator, bool>
                        (iterator(detail::ID_Forward,
                                  detail::IL_Regular,
                                  insert_result.second),
                         insert_result.first));
            } /*insert*/

            std::pair<iterator, bool>
            insert(std::pair<Key const, Value> && kv_pair) {
                using xo::scope;
                using xo::xtag;

                constexpr bool c_logging_enabled = false;
                scope log(XO_DEBUG(c_logging_enabled));

                RbNode * adj_root = this->root_;

                std::pair<bool, RbNode *> insert_result
                    = RbUtil::insert_aux(this->node_alloc_,
                                         std::move(kv_pair),
                                         true /*allow_replace_flag*/,
                                         this->reduce_fn_,
                                         this->debug_flag_,
                                         &adj_root);

                if (insert_result.first) {
                    ++(this->size_);

                    if (adj_root != root_) {
                        allocator_type alloc = node_alloc_;

                        this->_gc_assign_member(&(this->root_), adj_root, alloc);
                    }
                } else {
                    assert(adj_root == root_);
                }

                return (std::pair<iterator, bool>
                        (iterator(detail::ID_Forward,
                                  detail::IL_Regular,
                                  insert_result.second),
                         insert_result.first));
            } /*insert*/

            bool erase(Key const & key) {
                scope log(XO_DEBUG(debug_flag_), xtag("size", size_));
                if (log) {
                    log("pre", xtag("key", key), xtag("tree", *this));
                }

                bool retval = RbUtil::erase_aux(this->node_alloc_,
                                                key,
                                                this->reduce_fn_,
                                                debug_flag_,
                                                &(this->root_));

                if (retval)
                    --(this->size_);

                if (log) {
                    log("post", xtag("tree", *this));
                }

                return retval;
            } /*erase*/

            /* verify class invariants.
             * unless implementation is broken,  or client manages
             * to violate api rules,   this will always return true.
             *
             * RB0. if root node is nil then .size is 0
             * RB1. if root node is non-nil,  then root->parent() is nil,
             *      and .size = root->size
             * RB2. if N = P->child(d),  then N->parent()=P
             * RB3. all paths to leaves have the same black height
             * RB4. no red node has a red parent
             * RB5. inorder traversal visits keys in monotonically increasing order
             * RB6. Node::size reports the size of the subtree reachable from that node
             *      via child pointers
             * RB7. Node::reduced reports the value of
             *       f(f(L, Node::value), R)
             *      where: L is reduced-value for left child,
             *             R is reduced-value for right child
             * RB8. RedBlackTree.size() equals the #of nodes in tree
             */
            bool verify_ok(bool /*throw_flag_not_implemented*/ = true) const {
                using xo::scope;
                using xo::tostr;
                using xo::xtag;

                constexpr const char *c_self = "RedBlackTree::verify_ok";

                scope log(XO_DEBUG(debug_flag_));

                /* RB0. */
                if (root_ == nullptr) {
                    XO_EXPECT(size_ == 0, tostr(c_self, ": expect .size=0 with null root",
                                                xtag("size", size_)));
                }

                /* RB1. */
                if (root_ != nullptr) {
                    XO_EXPECT(root_->parent_ == nullptr,
                              tostr(c_self, ": expect root->parent=nullptr",
                                    xtag("parent", root_->parent_)));
                    XO_EXPECT(root_->size_ == this->size_,
                              tostr(c_self, ": expect self.size=root.size",
                                    xtag("self.size", size_),
                                    xtag("root.size", root_->size_)));
                }

                /* height (counting only black nodes) of tree */
                int32_t black_height = 0;

                /* n_node: #of nodes in this->root_ */
                size_t n_node = RbUtil::verify_subtree_ok(this->reduce_fn_,
                                                          this->root_,
                                                          &black_height);

                /* RB8. RedBlackTree.size() equals #of nodes in tree */
                XO_EXPECT(n_node == this->size_,
                          tostr(c_self, ": expect self.size={#of nodes n in tree}",
                                xtag("self.size", size_),
                                xtag("n", n_node)));

                if (debug_flag_)
                    log && log(xtag("size", this->size_),
                               xtag("blackheight", black_height));

                return true;
            } /*verify_ok*/

            void display_to_log() const { RbUtil::display(this->root_, 0); } /*display*/

            // ----- Inherited from GcObjectInterface -----

            virtual TaggedPtr self_tp() const {
                return Reflect::make_tp(const_cast<RedBlackTree *>(this));
            }
            virtual void display(std::ostream & os) const {
                os << "<RedBlackTree>";
            }
            virtual std::size_t _shallow_size() const final override { return sizeof(*this); }
            virtual IObject * _shallow_copy(gc::IAlloc * gc) const {
                if constexpr (GcObjectInterface::_requires_gc_hooks) {
                    xo::Cpof cpof(gc, this);
                    return new (cpof) RedBlackTree(*this);
                } else {
                    assert(false && "_shallow_copy assumes gc enabled");
                    return nullptr;
                }
            }
            virtual std::size_t _forward_children(gc::IAlloc * gc) {
                if constexpr (GcObjectInterface::_requires_gc_hooks) {
                    using xo::gc::ObjectVisitor;

                    ObjectVisitor<Reduce>::forward_children(reduce_fn_, gc);
                    gc->forward_inplace(reinterpret_cast<IObject **>(&root_));

                    return RedBlackTree::_shallow_size();
                } else {
                    assert(false && "_forward_children assumes gc enabled");
                    return 0ul;
                }
            }

        private:
            /** allocator state **/
            node_allocator_type node_alloc_;
            /** number of nodes in this tree. Each node holds one (key,value) pair **/
            size_t size_ = 0;
            /** accumulates custom order statistics;
             *  for example partial sums of @tparam Values
             *  reduce_fn_:: (Accumulator x Key) -> Accumulator
             **/
            Reduce reduce_fn_;
            /** root of red/black tree. Empty tree has null root. **/
            RbNode * root_ = nullptr;
            /** true to enable debug logging **/
            bool debug_flag_ = false;
        }; /*RedBlackTree*/

        template <typename Key,
                  typename Value,
                  typename Reduce,
                  typename Allocator>
        inline std::ostream &
        operator<<(std::ostream & os,
                   RedBlackTree<Key, Value, Reduce, Allocator> const & tree)
        {
            tree.display(os);
            return os;
        } /*operator<<*/

        template <typename Key,
                  typename Value,
                  typename Reduce,
                  typename GcObjectInterface,
                  bool IsConst>
        inline std::ostream &
        operator<<(std::ostream & os,
                   detail::IteratorBase<Key, Value, Reduce, GcObjectInterface, IsConst> const & iter)
        {
            iter.print(os);
            return os;
        } /*operator<<*/

    } /*namespace tree*/
} /*namespace xo*/

/* end RedBlackTree.hpp */
