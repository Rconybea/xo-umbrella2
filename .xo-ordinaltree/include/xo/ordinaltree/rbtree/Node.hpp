/** @file Node.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "RbTypes.hpp"
#include "xo/reflect/Reflect.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/allocutil/IAlloc.hpp"
#include "xo/allocutil/IObject.hpp"
#include "xo/allocutil/ObjectVisitor.hpp"
#include "xo/allocutil/gc_allocator_traits.hpp"
#include <concepts>
#include <cassert>
#include <utility>

namespace xo {
    namespace tree {
        namespace detail {
            /** see xo/ordinaltree/rbtree/RbTreeUtil.hpp */
            template <typename Key, typename Value,
                      typename Reduce, typename Compare,
                      typename GcObjectInterface>
            class RbTreeUtil;

            /* xo::tree::detail::Node
             *
             * Require:
             * - Key.operator<
             * - Key.operator==
             *
             */
            template <typename Key,
                      typename Value,
                      typename Reduce,
                      typename GcObjectInterface>
            requires valid_rbtree_node_params<Key, Value, Reduce, GcObjectInterface>
            class Node : public GcObjectInterface {
            public:
                using ReducedValue = typename Reduce::value_type;
                using ContentsType = std::pair<Key const, Value>;
                using value_type = std::pair<Key const , Value>;
                using rvpair_type = std::pair<ReducedValue, ReducedValue>;
                using Reflect = xo::reflect::Reflect;
                using TaggedPtr = xo::reflect::TaggedPtr;
                using IObject = xo::IObject;

            public:
                Node() = default;
                Node(value_type const & kv_pair,
                     rvpair_type const & rv_pair)
                    : color_(C_Red), size_(1), contents_{kv_pair}, reduced_(rv_pair) {}
                 Node(value_type && kv_pair,
                      rvpair_type && rv_pair)
                    : color_(C_Red), size_(1),
                      contents_{std::move(kv_pair)},
                      reduced_{std::move(rv_pair)} {}


                template <typename NodeAllocator>
                static Node * make_leaf(NodeAllocator& alloc,
                                        value_type const & kv_pair,
                                        ReducedValue const & leaf_rv) {
                    using traits = xo::gc::gc_allocator_traits<NodeAllocator>;

                    /* verify Node is constructible. instead of immediately attempting traits::construct */
                    static_assert(std::is_constructible_v<Node,
                                                          value_type const &,
                                                          rvpair_type const &>);

                    // get memory
                    Node * node = traits::allocate(alloc, 1);
                    try {
                        // placemenent new
                        traits::construct(alloc, node, kv_pair,
                                          rvpair_type(leaf_rv, leaf_rv));
                        return node;
                    } catch(...) {
                        traits::deallocate(alloc, node, 1);
                        throw;

                    }
                } /*make_leaf*/

                /* return #of key/vaue pairs in tree rooted at x. */
                static size_t tree_size(Node *x) {
                    if (x)
                        return x->size();
                    else
                        return 0;
                } /*tree_size*/

                static bool is_black(Node *x) {
                    if (x)
                        return x->is_black();
                    else
                        return true;
                } /*is_black*/

                static bool is_red(Node *x) {
                    if (x)
                        return x->is_red();
                    else
                        return false;
                } /*is_red*/

                static Direction child_direction(Node *p, Node *n) {
                    if (p) {
                        return p->child_direction(n);
                    } else {
                        return D_Invalid;
                    }
                } /*child_direction*/

                static ReducedValue reduce_aux(Reduce reduce, Node *x)
                    {
                        if(x)
                            return x->reduced2();
                        else
                            return reduce.nil();
                    } /*reduce_aux*/

                /* calculate reduced values for node x.
                 * does not used x.reduced
                 */
                static std::pair<ReducedValue,
                                 ReducedValue> reduced_pair(Reduce r, Node const * x)
                    {
                        assert(x != nullptr);

                        ReducedValue r1 = r(reduce_aux(r, x->left_child()),
                                            x->value());
                        ReducedValue r2 = r.combine(r1,
                                                    reduce_aux(r, x->right_child()));
                        return std::pair<ReducedValue, ReducedValue>(r1, r2);
                    } /*reduced_pair*/

                /* replace root pointer *pp_root with x;
                 * set x parent pointer to nil
                 */
                static void replace_root_reparent(Node * x, Node ** pp_root) {
                    *pp_root = x;
                    if (x)
                        x->parent_ = nullptr;
                } /*replace_root_reparent*/

                static void swap_color_size(Node * lhs,
                                            Node * rhs) {
                    std::swap(lhs->color_, rhs->color_);
                    std::swap(lhs->size_, rhs->size_);
                }

                /** swap values of all members except @ref contents_
                 *  between @p *lhs and @p *rhs
                 **/
                template <typename NodeAllocator>
                static void swap_locations(NodeAllocator & alloc,
                                           Node * lhs,
                                           Node * rhs,
                                           bool debug_flag) {
                    scope log(XO_DEBUG(debug_flag));

                    assert(lhs->parent() != rhs->parent());

                    Node * lhs_parent = lhs->parent();
                    Node * rhs_parent = rhs->parent();

                    /* can have null parent if either {lhs, rhs} is root node */

                    if (log) {
                        log("pre", xtag("lhs", lhs), xtag("rhs", rhs));
                        log(xtag("lhs.left", lhs->left_child()),
                            xtag("lhs.right", lhs->right_child()));
                        log(xtag("rhs.left", rhs->left_child()),
                            xtag("rhs.right", rhs->right_child()));
                    }

                    assert(lhs != rhs->left_child() && "not implemented");
                    assert(rhs != lhs->right_child() && "not implemented");
                    assert(rhs != lhs->left_child() && "expected left-to-right key order");
                    assert(lhs != rhs->right_child() && "expected left-to-right key order");

                    if (lhs_parent)
                        lhs_parent->replace_child_reparent(alloc, lhs, rhs);

                    /* now have:
                     * - rhs->parent() = lhs_parent
                     * - lhs->parent() = nullptr
                     */

                    if (rhs_parent)
                        rhs_parent->replace_child_reparent(alloc, rhs, lhs);

                    /* now have:
                     * - lhs->parent() = rhs_parent
                     * - rhs->parent() = lhs_parent
                     */

                    assert(lhs->parent() == rhs_parent);
                    assert(rhs->parent() == lhs_parent);
                    assert(lhs->parent() != lhs);
                    assert(rhs->parent() != rhs);

                    swap_color_size(lhs, rhs);

                    // preserve lhs->contents_, rhs->contents_
                    // don't bother fixing reduced_, will fixup that separately
                    //std::swap(lhs->reduced_, rhs->reduced_);

                    Node * lhs_left = lhs->left_child();
                    Node * lhs_right = lhs->right_child();

                    Node * rhs_left = rhs->left_child();
                    Node * rhs_right = rhs->right_child();

                    lhs->assign_child_reparent(alloc, D_Left, rhs_left);
                    lhs->assign_child_reparent(alloc, D_Right, rhs_right);
                    rhs->assign_child_reparent(alloc, D_Left, lhs_left);
                    rhs->assign_child_reparent(alloc, D_Right, lhs_right);

                    //std::swap(lhs->child_v_, rhs->child_v_);

                    /* also need to reparent */
                } /*swap_locations*/

                size_t size() const { return size_; }
                /* const access */
                value_type const & contents() const { return contents_; }
                /* non-const value access.
                 *
                 * editorial: would prefer to return
                 *   std::pair<Key const, Value> &
                 * here,  so that tree[k].first = newk
                 * prohibited,  but std::pair<Key const, Value>
                 * is considered unrelated to std::pair<Key, Value>,
                 * so l-value conversion not allowed
                 */
                value_type & contents() { return contents_; }

                Node * parent() const { return parent_; }
                Node * child(Direction d) const { return child_v_[d]; }
                Node * left_child() const { return child_v_[0]; }
                Node * right_child() const { return child_v_[1]; }
                ReducedValue const & reduced1() const { return reduced_.first; }
                ReducedValue const & reduced2() const { return reduced_.second; }

                /* true if this node has 0 children */
                bool is_leaf() const {
                    return ((child_v_[0] == nullptr) && (child_v_[1] == nullptr));
                }

                /* identify which child x represents
                 * Require:
                 * - x != nullptr
                 * - x is either this->left_child() or this->right_child()
                 */
                Direction child_direction(Node * x) const {
                    if (x == this->left_child())
                        return D_Left;
                    else if (x == this->right_child())
                        return D_Right;
                    else
                        return D_Invalid;
                } /*child_direction*/

                bool is_black() const { return this->color_ == C_Black; }
                bool is_red() const { return this->color_ == C_Red; }

                bool is_red_left() const { return is_red(this->left_child()); }
                bool is_red_right() const { return is_red(this->right_child()); }

                /* true if this node is red,  and either child is red */
                bool is_red_violation() const {
                    if (this->color_ == C_Red) {
                        Node *  left = this->left_child();
                        Node * right = this->right_child();

                        if (left && left->is_red())
                            return true;

                        if (right && right->is_red())
                            return true;
                    }

                    return false;
                } /*is_red_violation*/

                Color color() const { return color_; }
                Key const & key() const { return contents_.first; }
                Value const & value() const { return contents_.second; }

                void assign_color(Color x) { this->color_ = x; }
                void assign_size(size_t z) { this->size_ = z; }
                void assign_contents(const Value & x) { this->contents_.second = x; }
                Node * const * parent_addr() const { return &parent_; }
                Node * const * child_addr(Direction d) const { return &child_v_[d]; }

                /* recalculate size from immediate childrens' sizes
                 * editor bait: recalc_local_size()
                 */
                void local_recalc_size(Reduce const & reduce_fn) {
                    using xo::scope;
                    using xo::xtag;

                    constexpr bool c_logging_enabled = false;

                    scope log(XO_DEBUG(c_logging_enabled));

                    this->size_ = (1
                                   + Node::tree_size(this->left_child())
                                   + Node::tree_size(this->right_child()));

                    /* (note: want reduce applied to all of left subtree) */
                    this->reduced_ = Node::reduced_pair(reduce_fn, this);

                    log && log("done recalc for key k, value v, reduced r",
                               xtag("k", this->key()),
                               xtag("v", this->value()),
                               xtag("r1", this->reduced1()),
                               xtag("r2", this->reduced2()));
                } /*local_recalc_size*/

                // ----- inherited from GcObjectInterface -----

#ifdef NOPE
                virtual TaggedPtr self_tp() const {
                    return Reflect::make_tp(const_cast<Node *>(this));
                }
                virtual void display(std::ostream & os) const final override {
                    os << "<Node>";
                }
#endif

                virtual std::size_t _shallow_size() const final override { return sizeof(*this); }
                /* note: only relevant when GcObjectInterface is xo::IObject */
                virtual IObject * _shallow_copy(gc::IAlloc * gc) const final override {
                    if constexpr (GcObjectInterface::_requires_gc_hooks) {
                        xo::Cpof cpof(gc, this);
                        return new (cpof) Node(*this);
                    } else {
                        assert(false && "_shallow_copy assumes gc enabled");
                        return nullptr;
                    }
                }

                virtual std::size_t _forward_children(gc::IAlloc * gc) final override {
                    if constexpr (GcObjectInterface::_requires_gc_hooks) {
                        using xo::gc::ObjectVisitor;

                        static_assert(std::is_convertible_v<decltype(parent_), IObject *>,
                                      "parent_ must be convertible to IObject*");
                        static_assert(std::is_convertible_v<decltype(child_v_[0]), IObject *>,
                                      "child_v_[0] must be convertible to IObject*");

                        gc->forward_inplace(reinterpret_cast<IObject **>(&parent_));
                        gc->forward_inplace(reinterpret_cast<IObject **>(&child_v_[0]));
                        gc->forward_inplace(reinterpret_cast<IObject **>(&child_v_[1]));

                        /* for key, must cast away const so we can forward */
                        Key & key = const_cast<Key &>(contents_.first);
                        ObjectVisitor<Key>::forward_children(key, gc);

                        Value & value = contents_.second;
                        ObjectVisitor<Value>::forward_children(value, gc);

                        ReducedValue & rv1 = reduced_.first;
                        ObjectVisitor<ReducedValue>::forward_children(rv1, gc);

                        ReducedValue & rv2 = reduced_.second;
                        ObjectVisitor<ReducedValue>::forward_children(rv2, gc);

                        return Node::_shallow_size();
                    } else {
                        assert(false && "_forward_children assumes gc enabled");
                        return 0ul;
                    }
                }

                template <typename NodeAllocator>
                void assign_child_reparent(NodeAllocator & alloc,
                                           Direction d,
                                           Node * new_x)
                {
                    Node * old_x = this->child_v_[d];

                    using node_allocator_traits = xo::gc::gc_allocator_traits<NodeAllocator>;
                    using kvpair_allocator_type = node_allocator_traits::template rebind_alloc<value_type>;

                    kvpair_allocator_type assign_alloc(alloc);

                    // trying to fix old_x can be counterproductive,
                    // since old_x->parent_ may already have been corrected,
                    //
                    if (old_x && (old_x->parent_ == this)) {
                        old_x->parent_ = nullptr;
                    }

                    Node ** child_addr = &(this->child_v_[d]);
                    this->_gc_assign_member(child_addr, new_x, assign_alloc);
                    //this->child_v_[d] = new_x;

                    if (new_x) {
                        new_x->_gc_assign_member(&(new_x->parent_), this, assign_alloc);
                        //new_x->parent_ = this;
                    }
                } /*assign_child_reparent*/

                /* replace child that points to x,  with child that points to x_new
                 * and return direction of the child that was replaced
                 *
                 * Require:
                 * - x is a child of *this
                 * - x_new is not a child of *this
                 *
                 * promise:
                 * - x is nullptr or x.parent is nullptr
                 * - x_new is nullptr or x_new.parent is this
                 */
                template <typename NodeAllocator>
                Direction replace_child_reparent(NodeAllocator & alloc,
                                                   Node * x,
                                                   Node * x_new) {
                    Direction d = this->child_direction(x);

                    if ((d == D_Left) || (d == D_Right)) {
                        this->assign_child_reparent(alloc, d, x_new);
                        return d;
                    } else {
                        return D_Invalid;
                    }
                } /*replace_child_reparent*/

            private:
                /* red | black */
                Color color_ = C_Red;
                /* size of subtree (#of key/value pairs) rooted at this node */
                size_t size_ = 0;
                /* .first  = key   associated with this node (const!)
                 * .second = value associated with this node
                 */
                value_type contents_;
                /* accumulator for some binary function of Values.
                 * must be associative,  since value will be produced
                 * by any ordering of calls to Reduce::combine().
                 *
                 * e.g. {a, b, c, d} could be reduced by:
                 *   r(r(a,b), r(c,d))
                 * or
                 *   r(a, r(r(b, c), d))
                 * etc.
                 *
                 * examples:
                 *  - count #of keys
                 *  - sum key values
                 *
                 * .reduced.first:  reduce applied to all values with keys <= .contents.first
                 * .reduced.second: reduce applied to all values in this subtree.
                 */
                std::pair<ReducedValue, ReducedValue> reduced_;
                /* pointer to parent node,  nullptr iff this is the root node */
                Node * parent_ = nullptr;
                /*
                 * .child_v[0] = left child
                 * .child_v[1] = right child
                 *
                 * invariants:
                 * - if .child_v[x] non-null,  then .child_v[0]->parent = this
                 * - a red node may not have red children
                 */
                std::array<Node *, 2> child_v_ = {nullptr, nullptr};
            }; /*Node*/

        } /*namespace detail*/
    } /*namespace tree*/
} /*namespace xo*/


/* end Node.hpp */
