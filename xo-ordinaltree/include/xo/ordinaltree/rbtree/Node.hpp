/** @file Node.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "RbTypes.hpp"
#include "xo/allocutil/gc_allocator_traits.hpp"
#include <utility>

namespace xo {
    namespace tree {
        namespace detail {
            /** see xo/ordinaltree/rbtree/RbTreeUtil.hpp */
            template <typename Key, typename Value, typename Reduce>
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
                      typename Reduce>
            class Node {
            public:
                using ReducedValue = typename Reduce::value_type;
                using ContentsType = std::pair<Key const, Value>;
                using value_type = std::pair<Key const , Value>;

            public:
                Node() = default;
                Node(value_type const & kv_pair,
                     std::pair<ReducedValue, ReducedValue> const & r)
                    : color_(C_Red), size_(1), contents_{kv_pair}, reduced_(r) {}
                Node(value_type && kv_pair,
                     std::pair<ReducedValue, ReducedValue> && r)
                    : color_(C_Red), size_(1),
                      contents_{std::move(kv_pair)},
                      reduced_{std::move(r)} {}

                template <typename NodeAllocator>
                static Node * make_leaf(NodeAllocator& alloc,
                                        value_type const & kv_pair,
                                        ReducedValue const & leaf_rv) {
                    using traits = std::allocator_traits<NodeAllocator>;

                    // get memory
                    Node * node = traits::allocate(alloc, 1);
                    try {
                        // placemenent new
                        traits::construct(alloc, node, kv_pair,
                                          std::pair<ReducedValue, ReducedValue>(leaf_rv, leaf_rv));
                        return node;
                    } catch(...) {
                        traits::deallocate(alloc, node, 1);
                        throw;

                    }
                } /*make_leaf*/

#ifdef OBSOLETE
                static Node * make_leaf(value_type && kv_pair,
                                        ReducedValue const & leaf_rv) {
                    return new Node(kv_pair,
                                    std::pair<ReducedValue, ReducedValue>(leaf_rv, leaf_rv));
                } /*make_leaf*/
#endif

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
                        if(!x)
                            assert(false);

                        ReducedValue r1 = r(reduce_aux(r, x->left_child()),
                                            x->value());
                        ReducedValue r2 = r.combine(r1,
                                                    reduce_aux(r, x->right_child()));
                        return std::pair<ReducedValue, ReducedValue>(r1, r2);
                    } /*reduced_pair*/

                /* replace root pointer *pp_root with x;
                 * set x parent pointer to nil
                 */
                static void replace_root_reparent(Node *x, Node **pp_root) {
                    *pp_root = x;
                    if (x)
                        x->parent_ = nullptr;
                } /*replace_root_reparent*/

                /** swap values of all members except @ref contents_
                 *  between @p *lhs and @p *rhs
                 **/
                static void swap_locations(Node * lhs, Node * rhs, bool debug_flag) {
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
                        lhs_parent->replace_child_reparent(lhs, rhs);

                    /* now have:
                     * - rhs->parent() = lhs_parent
                     * - lhs->parent() = nullptr
                     */

                    if (rhs_parent)
                        rhs_parent->replace_child_reparent(rhs, lhs);

                    /* now have:
                     * - lhs->parent() = rhs_parent
                     * - rhs->parent() = lhs_parent
                     */

                    assert(lhs->parent() == rhs_parent);
                    assert(rhs->parent() == lhs_parent);
                    assert(lhs->parent() != lhs);
                    assert(rhs->parent() != rhs);

                    std::swap(lhs->color_, rhs->color_);
                    std::swap(lhs->size_, rhs->size_);
                    // preserve lhs->contents_, rhs->contents_
                    // don't bother fixing reduced_, will fixup that separately
                    //std::swap(lhs->reduced_, rhs->reduced_);

                    Node * lhs_left = lhs->left_child();
                    Node * lhs_right = lhs->right_child();

                    Node * rhs_left = rhs->left_child();
                    Node * rhs_right = rhs->right_child();

                    lhs->assign_child_reparent(D_Left, rhs_left);
                    lhs->assign_child_reparent(D_Right, rhs_right);
                    rhs->assign_child_reparent(D_Left, lhs_left);
                    rhs->assign_child_reparent(D_Right, lhs_right);

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
                        Node *left = this->left_child();
                        Node *right = this->right_child();

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

            private:
                void assign_color(Color x) { this->color_ = x; }
                void assign_size(size_t z) { this->size_ = z; }

                void assign_child_reparent(Direction d, Node *new_x) {
                    Node *old_x = this->child_v_[d];

                    // trying to fix old_x can be counterproductive,
                    // since old_x->parent_ may already have been corrected,
                    //
                    if (old_x && (old_x->parent_ == this))
                        old_x->parent_ = nullptr;

                    this->child_v_[d] = new_x;

                    if (new_x) {
                        new_x->parent_ = this;
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
                Direction replace_child_reparent(Node *x, Node *x_new) {
                    Direction d = this->child_direction(x);

                    if (d == D_Left || d == D_Right) {
                        this->assign_child_reparent(d, x_new);
                        return d;
                    } else {
                        return D_Invalid;
                    }
                } /*replace_child_reparent*/

                friend class RbTreeUtil<Key, Value, Reduce>;
                friend class xo::tree::RedBlackTree<Key, Value, Reduce>;

            private:
                /* red | black */
                Color color_ = C_Red;
                /* size of subtree (#of key/value pairs) rooted at this node */
                size_t size_ = 0;
                /* .first  = key   associated with this node
                 * .second = value associated with this node
                 * .third  = reduced value
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
                Node *parent_ = nullptr;
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
