/** @file RbTreeUtil.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "Node.hpp"
#include "xo/indentlog/scope.hpp"

namespace xo {
    namespace tree {
        namespace detail {
            /* require:
             * - Reduce::value_type
             *
             * matches forward tmplate decl in Node.hpp
             */
            template <typename Key,
                      typename Value,
                      typename Reduce>
            class RbTreeUtil {
            public:
                using RbNode = Node<Key, Value, Reduce>;
                using ReducedValue = typename Reduce::value_type;
                using value_type = std::pair<Key const, Value>;

            public:
                /* return #of key/vaue pairs in tree rooted at x. */
                static size_t tree_size(RbNode *x) {
                    if (x)
                        return x->size();
                    else
                        return 0;
                } /*tree_size*/

                static bool is_black(RbNode *x) {
                    if (x)
                        return x->is_black();
                    else
                        return true;
                } /*is_black*/

                static bool is_red(RbNode *x) {
                    if (x)
                        return x->is_red();
                    else
                        return false;
                } /*is_red*/

                /* for every node n in tree, call fn(n, d').
                 * d' is the depth of the node n relative to starting point x,
                 * not counting red nodes.
                 * make calls in increasing key order (i.e. inorder traversal)
                 * argument d is the black-height of tree above x
                 *
                 * Require:
                 * - fn(x, d)
                 */
                template <typename Fn>
                static void inorder_node_visitor(RbNode const * x, uint32_t d, Fn && fn) {
                    if (x) {
                        /* dd: black depth of child subtrees*/
                        uint32_t dd = (x->is_black() ? d + 1 : d);

                        inorder_node_visitor(x->left_child(), dd, fn);
                        /* dd includes this node */
                        fn(x, dd);
                        inorder_node_visitor(x->right_child(), dd, fn);
                    }
                } /*inorder_node_visitor*/

                /* note: RedBlackTree.clear() abuses this to visit-and-delete
                 *       all nodes
                 */
                template <typename Fn>
                static void postorder_node_visitor(RbNode const * x, uint32_t d, Fn && fn) {
                    if (x) {
                        uint32_t dd = (x->is_black() ? d + 1 : d);

                        postorder_node_visitor(x->left_child(), dd, fn);
                        postorder_node_visitor(x->right_child(), dd, fn);
                        /* dd includes this node */
                        fn(x, dd);
                    }
                } /*postorder_node_visitor*/

                /* return the i'th inorder node (counting from 0)
                 * belonging to the subtree rooted at N.
                 *
                 * behavior not defined if subtree at N contains less than
                 * (i + 1) nodes
                 */
                static RbNode * find_ith(RbNode * N, uint32_t i) {
                    if(!N)
                        return nullptr;

                    RbNode * L = N->left_child();
                    uint32_t n_left = tree_size(L);

                    if(i < n_left)
                        return find_ith(L, i);
                    else if(i == n_left)
                        return N;
                    else if(i < N->size_)
                        return find_ith(N->right_child(), i - (n_left + 1));
                    else
                        return nullptr;
                } /*find_ith*/

                /* starting from x,  traverse only left children
                 * to find node with a nil left child.
                 *
                 * This node has the smallest key in subtree N
                 */
                static RbNode * find_leftmost(RbNode * N) {
                    while(N) {
                        RbNode * S = N->left_child();

                        if(!S)
                            break;

                        N = S;
                    }

                    return N;
                } /*find_leftmost*/

                /* return node containing the next key after N->key_ in the tree
                 * containing N.  This will be either a descendant of N,
                 * or an ancestor of N.
                 * returns nil if x.key is the largest key in tree containing x.
                 */
                static RbNode * next_inorder_node(RbNode * N) {
                    if(!N)
                        return nullptr;

                    if(N->right_child())
                        return find_leftmost(N->right_child());

                    /* N has no right child -->
                     * successor is the nearest ancestor with a left child
                     * on path to N
                     */

                    RbNode * x = N;

                    while(x) {
                        RbNode * P = x->parent();

                        if(P && P->left_child() == x) {
                            return P;
                        }

                        /* path P..N traverses only right-child pointers) */
                        x = P;
                    }

                    /* no ancestor of N with a left child,  so N has the largest key
                     * in the tree
                     */
                    return nullptr;
                } /*next_inorder_node*/

                /* return node containing the key before N->key_ in the tree containing N.
                 * This will be either a descendant of N, or an ancestor of N
                 */
                static RbNode * prev_inorder_node(RbNode * N) {
                    if(!N)
                        return nullptr;

                    if(N->left_child())
                        return find_rightmost(N->left_child());

                    /* N has no left child -->
                     * predecessor is the nearest ancestor with a right child
                     * on path to N
                     */

                    RbNode * x = N;

                    while(x) {
                        RbNode * P = x->parent();

                        if(P && (P->right_child() == x)) {
                            return P;
                        }

                        /* path P..N traverses only left-child pointers */
                        x = P;
                    }

                    /* no ancestor of N with a right child,  so N has the smallest key
                     * in tree that containing it.
                     */
                    return nullptr;
                } /*prev_inorder_node*/

                /* compute value of reduce applied to the set K of all keys k[j] in subtree N
                 * with:
                 *   k[j] <= lub_key  if is_closed = true
                 *   k[j] <  lub_key  if is_closed = false
                 * return reduce_fn.nil() if K is empty
                 */
                static ReducedValue reduce_lub(Key const & lub_key,
                                               Reduce const & reduce_fn,
                                               bool is_closed,
                                               RbNode * N)
                    {
                        ReducedValue retval = reduce_fn.nil();

                        for (;;) {
                            if (!N)
                                return retval;

                            if ((N->key() < lub_key) || (is_closed && (N->key() == lub_key))) {
                                /* all keys k[i] in left subtree of N satisfy k[i] < lub_key
                                 * apply reduce to:
                                 * - left subtree of N
                                 * - N->key() depending on comparison with lub_key
                                 * - any members of right subtree of N, with key < lub_key;
                                 */
                                retval = reduce_fn.combine(retval, N->reduced1());
                                N = N->right_child();
                            } else {
                                /* all keys k[j] in right subtree of N do NOT satisfy k[j] <
                                 * lub_key, exclude these.   also exclude N->key()
                                 */
                                N = N->left_child();
                            }
                        }
                    } /*reduce_lub*/

                /* find largest key k such that
                 *   reduce({node j in subtree(N)) | j.key <= k}) < p
                 *
                 *   ^
                 * 1 |           xxxx
                 *   |         xx
                 * p |....... x
                 *   |       x
                 *   |     xx .
                 *   | xxxx   .
                 * 0 +---------------->
                 *            ^
                 *            find_cum_glb(p)
                 *
                 * here Key is a sample value,
                 * Value counts #of samples with that key.
                 *
                 * find_cum_glb() computes inverse for a monotonically increasing function,
                 * if reduce(S) = sum {j.value | j in S};
                 *
                 * if rbtree stores values for a discrete function f: IR -> IR+,
                 * then x = find_sum_glb(p)->key() inverts the integral of f,  i.e.
                 * computes:
                 *             x
                 *            /
                 *            |
                 *   sup { x: |  f(z) dz < y }
                 *            |
                 *            /
                 *             -oo
                 *
                 * Require:
                 * - Reduce behaves like sum:
                 *   must deliver monotonically increasing values
                 *   with increasing key-values.
                 *
                 *   (for example: if Value is non-negative and Reduce is SumReduce<Value>)
                 */
                static RbNode * find_sum_glb(Reduce const & reduce_fn,
                                             RbNode * N,
                                             typename Reduce::value_type y) {
                    using xo::scope;
                    using xo::xtag;

                    constexpr char const * c_self = "RbTreeUtil::find_sum_glb";
                    constexpr bool c_logging_enabled = false;
                    scope log(XO_DEBUG(c_logging_enabled));

                    if(!N) {
                        log && log(c_self, ": return nullptr");
                        return nullptr;
                    }

                    typename Reduce::value_type left_sum
                        = RbNode::reduce_aux(reduce_fn, N->left_child());
                    typename Reduce::value_type right_sum
                        = RbNode::reduce_aux(reduce_fn, N->right_child());

                    log && log("with",
                               xtag("y", y),
                               xtag("N.key", N->key()),
                               xtag("N.value", N->value()),
                               xtag("N.reduced1", N->reduced1()),
                               xtag("left_sum", left_sum),
                               xtag("right_sum", right_sum));

                    if (y <= left_sum) {
                        return find_sum_glb(reduce_fn, N->left_child(), y);
                    } else if (y <= N->reduced1() || !N->right_child()) {
                        log && log("return N");
                        /* since N.reduced = reduce(left_sum, N.value, right_sum) */
                        return N;
                    } else {
                        /* find bound in non-null right subtree */
                        return find_sum_glb(reduce_fn, N->right_child(), y - N->reduced1());
                    }
                } /*find_sum_glb*/

                /* starting from x,  traverse only right children
                 * to find node with a nil right child
                 *
                 * This node has the largest key in subtree N
                 */
                static RbNode * find_rightmost(RbNode *N) {
                    while(N) {
                        RbNode *S = N->right_child();

                        if (!S)
                            break;

                        N = S;
                    }

                    return N;
                } /*find_rightmost*/

                /* find node in x with key k
                 * return nullptr iff no such node exists.
                 */
                static RbNode * find(RbNode * x, Key const & k) {
                    for (;;) {
                        if (!x)
                            return nullptr;

                        if (k < x->key()) {
                            /* search in left subtree */
                            x = x->left_child();
                        } else if (k == x->key()) {
                            return x;
                        } else /* k > x->key() */ {
                            x = x->right_child();
                        }
                    }
                } /*find*/

                /* find greatest lower bound for key k in tree x,
                 * provided it's tighter than candidate h.
                 *
                 * require:
                 * if h is provided,  then x belongs to right subtree of h
                 * (so any key k' in x satisfies k' > h->key)
                 *
                 */
                static RbNode *find_glb_aux(RbNode *x, RbNode *h, Key const &k,
                                            bool is_closed) {
                    for (;;) {
                        if (!x)
                            return h;

                        if (x->key() < k) {
                            /* x.key is a lower bound for k */

                            if (x->right_child() == nullptr) {
                                /* no tighter lower bounds present in subtree rooted at x */

                                /* x must be better lower bound than h,
                                 * since when h is non-nil we are searching right subtree of h
                                 */
                                return x;
                            }

                            /* look for better lower bound in right child */
                            h = x;
                            x = x->right_child();
                            continue;
                        } else if (is_closed && (x->key() == k)) {
                            /* x.key is exact match */
                            return x;
                        } else {
                            /* x.key is an upper bound for k.  If there's a lower bound,
                             * it must be in left subtree of x
                             */

                            /* preserving h */
                            x = x->left_child();
                            continue;
                        }
                    } /*looping over tree nodes*/
                }   /*find_glb_aux*/

                /* find greatest lower bound node for a key,  in this subtree
                 *
                 * is_open.  if true,  allow result with N->key = k exactly
                 *           if false,  require N->key < k
                 */
                static RbNode * find_glb(RbNode * x, Key const & k, bool is_closed) {
                    return find_glb_aux(x, nullptr, k, is_closed);
                } /*find_glb*/

#ifdef NOT_IN_USE
                /* find least upper bound node for a key,  in this subtree*
                 *
                 * is_open.  if true,  allow result with N->key = k exactly
                 *           if false,  require N->key > k
                 */
                static RbNode *find_lub(RbNode *x, Key const &k, bool is_closed) {
                    if (x->key() > k) {
                        /* x.key is an upper bound for k */
                        if (x->left_child() == nullptr) {
                            /* no tigher upper bound present in subtree rooted at x */
                            return x;
                        }

                        RbNode *y = find_lub(x->left_child(), k, is_closed);

                        if (y) {
                            /* found better upper bound in left subtree */
                            return y;
                        } else {
                            return x;
                        }
                    } else if (is_closed && (x->key() == k)) {
                        return x;
                    } else {
                        /* x.key is not an upper bound for k */
                        return find_lub(x->right_child(), k, is_closed);
                    }
                } /*find_lub*/
#endif

                /* perform a tree rotation in direction d at node A.
                 *
                 * Require:
                 * - A is non-nil
                 * - A->child(other(d)) is non-nil
                 *
                 * if direction=D_Left:
                 *
                 *        G                 G
                 *        |                 |
                 *        A                 B  <- retval
                 *       / \               / \
                 *      R   B      ==>    A   T
                 *         / \           / \
                 *        S   T         R   S
                 *
                 * if direction=D_Right:
                 *
                 *        G                  G
                 *        |                  |
                 *        A                  B <- retval
                 *       / \                / \
                 *      B   R        ==>   T   A
                 *     / \                    / \
                 *    T   S                  S   R
                 */
                static RbNode *rotate(Direction d, RbNode *A,
                                      Reduce const & reduce_fn,
                                      RbNode **pp_root) {
                    using xo::scope;
                    using xo::xtag;

                    //constexpr char const *c_self = "RbTreeUtil::rotate";
                    constexpr bool c_logging_enabled = false;

                    scope log(XO_DEBUG(c_logging_enabled));

                    Direction other_d = other(d);

                    RbNode *G = A->parent();
                    RbNode *B = A->child(other_d);
                    //RbNode *R = A->child(d);        // not using
                    RbNode *S = B->child(d);
                    //RbNode *T = B->child(other_d);  // not using

                    if (log.enabled()) {
                        log("rotate-", (d == D_Left) ? "left" : "right",
                            " at", xtag("A", A), xtag("A.key", A->key()), xtag("B", B),
                            xtag("B.key", B->key()));

                        if (G) {
                            log("with G", xtag("G", G),
                                xtag("G.key", G->key()));
                            // display_aux(D_Invalid /*side*/, G, 0, &lscope);
                        } else {
                            log("with A at root");
                            // display_aux(D_Invalid /*side*/, A, 0, &lscope);
                        }
                    }

                    /* note: this will set A's old child B to have null parent ptr */
                    A->assign_child_reparent(other_d, S);
                    A->local_recalc_size(reduce_fn);

                    B->assign_child_reparent(d, A);
                    B->local_recalc_size(reduce_fn);

                    if (G) {
                        G->replace_child_reparent(A, B);
                        assert(B->parent() == G);

                        /* note: G.size not affected by rotation */
                    } else {
                        RbNode::replace_root_reparent(B, pp_root);
                    }

                    return B;
                } /*rotate*/

                /* fixup size in N and all ancestors of N,
                 * after insert/remove affecting N
                 */
                static void fixup_ancestor_size(Reduce const & reduce_fn, RbNode *N, bool debug_flag) {
                    scope log(XO_DEBUG(debug_flag));
                    size_t depth = 0;
                    for (; N && depth < 128; ++depth) {
                        log && log("fixup size: ", xtag("N", N), xtag("ancestors", depth));
                        N->local_recalc_size(reduce_fn);
                        N = N->parent();
                    }

                    if (N) {
                        throw std::runtime_error("fixup_ancestor_size: excessive depth -> infinite loop?");
                    }
                } /*fixup_ancestor_size*/

                /* rebalance to fix possible red-red violation at node G or G->child(d).
                 *
                 * diagrams are for d=D_Left;
                 * mirror left-to-right to get diagram for d=D_Right
                 *
                 *             G
                 *        d-> / \ <-other_d
                 *           P   U
                 *          / \
                 *         R   S
                 *
                 * relative to prevailing black-height h:
                 * - P at h
                 * - U at h
                 * - may have red-red violation between G and P
                 *
                 * Require:
                 * - tree is in RB-shape,  except for possible red-red violation
                 *   between {G,P} or {P,R|S}
                 * Promise:
                 * - tree is in RB-shape
                 */
                static void fixup_red_shape(Direction d, RbNode *G,
                                            Reduce const & reduce_fn,
                                            RbNode **pp_root) {
                    using xo::scope;
                    using xo::xtag;
                    using xo::print::ccs;

                    //constexpr char const *c_self = "RbTreeUtil::fixup_red_shape";
                    constexpr bool c_logging_enabled = false;
                    constexpr bool c_excessive_verify_enabled = false;

                    scope log(XO_DEBUG(c_logging_enabled));

                    RbNode *P = G->child(d);

                    for (uint32_t iter = 0;; ++iter) {
                        if (c_excessive_verify_enabled)
                            RbTreeUtil::verify_subtree_ok(reduce_fn, G, nullptr /*&black_height*/);

                        if (log.enabled()) {
                            if (G) {
                                log("consider node G with d-child P",
                                    xtag("iter", iter), xtag("G", G),
                                    xtag("G.col", ccs((G->color() == C_Red) ? "r" : "B")),
                                    xtag("G.key", G->key()),
                                    xtag("d", ccs((d == D_Left) ? "L" : "R")),
                                    xtag("P", P),
                                    xtag("P.col", ccs((P->color() == C_Red) ? "r" : "B")),
                                    xtag("P.key", P->key()));
                            } else {
                                log("consider root P", xtag("iter", iter),
                                    xtag("P", P),
                                    xtag("P.col", ccs((P->color() == C_Red) ? "r" : "B")),
                                    xtag("P.key", P->key()));
                            }

                            RbTreeUtil::display_aux(D_Invalid /*side*/, G ? G : P, 0 /*d*/,
                                                    &log);
                        } /*if logging enabled*/

                        if (G && G->is_red_violation()) {
                            log && log("red-red violation at G - defer");

                            /* need to fix red-red violation at next level up
                             *
                             *       .  (=G')
                             *       |  (=d')
                             *       G* (=P')
                             *  d-> / \ <-other-d
                             *     P*  U
                             *    / \
                             *   R   S
                             */
                            P = G;
                            G = G->parent();
                            d = RbNode::child_direction(G, P);

                            continue;
                        }

                        log && log("check for red violation at P");

                        if (!P->is_red_violation()) {
                            log && log("red-shape ok at {G,P}");

                            /* RB-shape restored */
                            return;
                        }

                        if (!G) {
                            log && log("make P black to fix red-shape at root");

                            /* special case:  P is root of tree.
                             * can fix red violation by making P black
                             */
                            P->assign_color(C_Black);
                            return;
                        }

                        Direction other_d = other(d);

                        RbNode *R = P->child(d);
                        RbNode *S = P->child(other_d);
                        RbNode *U = G->child(other_d);

                        if (log.enabled()) {
                            log("got R,S,U", xtag("R", R), xtag("S", S),
                                xtag("U", U));
                            if (R) {
                                log("with",
                                    xtag("R.col", ccs(R->color_ == C_Black ? "B" : "r")),
                                    xtag("R.key", R->key()));
                            }
                            if (S) {
                                log("with",
                                    xtag("S.col", ccs(S->color_ == C_Black ? "B" : "r")),
                                    xtag("S.key", S->key()));
                            }
                            if (U) {
                                log("with",
                                    xtag("U.col", ccs(U->color_ == C_Black ? "B" : "r")),
                                    xtag("U.key", U->key()));
                            }
                        }

                        assert(is_black(G));
                        assert(is_red(P));
                        assert(is_red(R) || is_red(S));

                        if (RbNode::is_red(U)) {
                            /* if d=D_Left:
                             *
                             *   *=red node
                             *
                             *           .                 .  (=G')
                             *           |                 |  (=d')
                             *           G                 G* (=P')
                             *      d-> / \               / \
                             *         P*  U*   ==>      P   U
                             *        / \               / \
                             *    (*)R   S(*)       (*)R   S(*)
                             *
                             * (*) exactly one of R or S is red (since we have a red-violation
                             * at P)
                             *
                             * Note: this transformation preserves #of black nodes along path
                             * from root to each of {T, R, S},  so it preserves the "equal
                             * black-node path" property
                             */
                            G->assign_color(C_Red);
                            P->assign_color(C_Black);
                            U->assign_color(C_Black);

                            log && log("fixed red violation at P, retry 1 level higher");

                            /* still need to check for red-violation at G's parent */
                            P = G;
                            G = G->parent();
                            d = RbNode::child_direction(G, P);

                            continue;
                        }

                        assert(RbNode::is_black(U));

                        if (RbNode::is_red(S)) {
                            log && log("rotate-", (d == D_Left) ? "left" : "right",
                                       " at P", xtag("P", P), xtag("P.key", P->key()),
                                       xtag("S", S), xtag("S.key", S->key()));

                            /* preparatory step: rotate P in d direction if "inner child"
                             * (S) is red inner-child = right-child of left-parent or vice
                             * versa
                             *
                             *        G                      G
                             *       / \                    / \
                             *      P*  U     ==>    (P'=) S*  U
                             *     / \                    / \
                             *    R   S*           (R'=) P*
                             *                          / \
                             *                         R
                             */
                            RbTreeUtil::rotate(d, P, reduce_fn, pp_root);

                            if (c_excessive_verify_enabled)
                                RbTreeUtil::verify_subtree_ok(reduce_fn, S, nullptr /*&black_height*/);

                            /* (relabel S->P etc. for merged control flow below) */
                            R = P;
                            P = S;
                        }

                        /*
                         *        G                P
                         *       / \              / \
                         *      P*  U     ==>    R*  G*
                         *     / \                  / \
                         *    R*  S                S   U
                         *
                         * ok since every path that went through previously-black G
                         * now goes through newly-black P
                         */
                        P->assign_color(C_Black);
                        G->assign_color(C_Red);

                        log && log("rotate-",
                                   (other_d == D_Left) ? "left" : "right", " at G",
                                   xtag("G", G), xtag("G.key", G->key()));

                        RbTreeUtil::rotate(other_d, G, reduce_fn, pp_root);

                        if (c_excessive_verify_enabled) {
                            RbNode *GG = G ? G->parent() : G;
                            if (!GG)
                                GG = P;

                            if (log.enabled()) {
                                log("verify subtree at GG", xtag("GG", GG),
                                    xtag("GG.key", GG->key()));

                                RbTreeUtil::verify_subtree_ok(reduce_fn, GG, nullptr /*&black_height*/);
                                RbTreeUtil::display_aux(D_Invalid, GG, 0 /*depth*/, &log);

                                log("fixup complete");
                            }
                        }

                        return;
                    } /*walk toward root until red violation fixed*/
                }   /*fixup_red_shape*/

                /* insert key-value pair (key, value) into *pp_root.
                 * on exit *pp_root contains new tree with (key, value) inserted.
                 * returns true if node was inserted,  false if instead an existing node
                 * with the same key was replaced.
                 *
                 * Require:
                 * - pp_root is non-nil  (*pp_root may be nullptr -> empty tree)
                 * - *pp_root is in RB-shape
                 *
                 * allow_replace_flag.   if true,  v will replace an existing value
                 *                       associated with key k.
                 *                       if false,  preserve existing value.
                 *                       when k already exists in *pp_root.
                 *
                 * return pair<f,n> with:
                 * - f=true for new node (k did not exist in tree before this call)
                 * - f=false for existing node (k already in tree before this call)
                 * - n=node containing key k
                 */
                template <typename NodeAllocator>
                static std::pair<bool, RbNode *>
                insert_aux(NodeAllocator & alloc,
                           value_type const & kv_pair,
                           bool allow_replace_flag,
                           Reduce const & reduce_fn,
                           RbNode ** pp_root)
                    {
                        using xo::xtag;

                        constexpr bool c_debug_flag = false;
                        //XO_SCOPE2(log, true /*debug_flag*/);

                        RbNode * N = *pp_root;

                        Direction d = D_Invalid;

                        while (N) {
                            if (kv_pair.first == N->key()) {
                                if(allow_replace_flag) {
                                    /* match on this key already present in tree
                                     *  -> just update assoc'd value
                                     */
                                    N->contents_.second = kv_pair.second;
                                }

                                /* after modifying a node n,  must recalculate reductions
                                 * along path [root .. n]
                                 */
                                RbTreeUtil::fixup_ancestor_size(reduce_fn, N, c_debug_flag);

                                //log && log(xtag("path", (char const *)"A"));

                                /* since we didn't change the set of nodes,
                                 * tree is still in RB-shape,  don't need to call fixup_red_shape()
                                 */
                                return std::make_pair(false, N);
                            }

                            d = ((kv_pair.first < N->key()) ? D_Left : D_Right);

                            /* insert into left subtree somewhere */
                            RbNode *C = N->child(d);

                            if (!C)
                                break;

                            N = C;
                        }

                        /* invariant: N->child(d) is nil */

                        if (N) {
                            RbNode * new_node = RbNode::make_leaf(alloc,
                                                                  kv_pair,

                                                                  reduce_fn.leaf(kv_pair.second));

                            N->assign_child_reparent(d, new_node);

                            assert(is_red(N->child(d)));

                            /* recalculate Node sizes on path [root .. N] */
                            RbTreeUtil::fixup_ancestor_size(reduce_fn, N, c_debug_flag);
                            /* after adding a node,  must rebalance to restore RB-shape */
                            RbTreeUtil::fixup_red_shape(d, N, reduce_fn, pp_root);

                            //log && log(xtag("path", (char const *)"B"));

                            /* note: new_node=N.child(d) is true before call to fixup_red_shape(),
                             *       but not necessarily after
                             */
                            return std::make_pair(true, new_node);
                        } else {
                            *pp_root = RbNode::make_leaf(alloc,
                                                         kv_pair,
                                                         reduce_fn.leaf(kv_pair.second));

                            /* tree with a single node might as well be black */
                            (*pp_root)->assign_color(C_Black);

                            //(*pp_root)->local_recalc_size(reduce_fn);

                            /* Node.size will be correct for tree,  since
                             * new node is only node in the tree
                             */

                            //log && log(xtag("path", (char const *)"C"));

                            return std::make_pair(true, *pp_root);
                        }

                    } /*insert_aux*/

                /* remove a black node N with no children.
                 * this will reduce black-height along path to N
                 * by 1,   so will need to rebalance tree
                 *
                 * pp_root.  pointer to location of tree root;
                 *           may update with new root
                 *
                 * Require:
                 * - N != nullptr
                 * - N has no child nodes
                 * - N->parent() != nullptr
                 */
                template <typename NodeAllocator>
                static void remove_black_leaf(NodeAllocator & alloc,
                                              RbNode *N,
                                              Reduce const & reduce_fn,
                                              bool debug_flag,
                                              RbNode **pp_root)
                    {
                        using xo::scope;
                        using xo::xtag;
                        using xo::print::ccs;

                        using traits = std::allocator_traits<NodeAllocator>;

                        //constexpr char const *c_self = "RbTreeUtil::remove_black_leaf";

                        scope log(XO_DEBUG(debug_flag));

                        assert(pp_root);

                        RbNode *P = N->parent();

                        if (!P) {
                            /* N was the root node,  tree now empty */
                            *pp_root = nullptr;
                            traits::deallocate(alloc, N, 1);
                            return;
                        }

                        /* d: direction in P to immediate child N;
                         * also sets N.parent to nil
                         */
                        Direction d = P->replace_child_reparent(N, nullptr);

                        traits::deallocate(alloc, N, 1);

                        /* need to delay this assignment until
                         * we've determined d
                         */
                        N = nullptr;

                        /* fixup sizes on path root..P
                         * subsequent rebalancing rotations will preserve correct .size values
                         */
                        RbTreeUtil::fixup_ancestor_size(reduce_fn, P, debug_flag);

                        /* other_d, S, C, D will be assigned by loop below
                         *
                         * diagram shown with d=D_Left;  mirror left-to-right for d=D_Right
                         *
                         *       P
                         *  d-> / \ <-other_d
                         *     N   S
                         *        / \
                         *       C   D
                         */
                        Direction other_d;
                        RbNode *S = nullptr;
                        RbNode *C = nullptr;
                        RbNode *D = nullptr;

                        /* table of outcomes as a function of node color
                         *
                         * .=black
                         * *=red
                         * x=don't care
                         *
                         * #=#of combinations (/16) for P,S,C,D color explained by this row
                         *
                         *  P  S  C  D  case     #
                         * -----------------------
                         *  .  .  .  .  Case(1)  1
                         *  x  *  x  x  Case(3)  8  P,C,D black is forced by RB rules
                         *  *  .  .  .  Case(4)  1
                         *  x  .  *  .  Case(5)  2
                         *  x  .  x  *  Case(6)  4
                         *                      --
                         *                      16
                         *
                         */

                        while (true) {
                            assert(is_black(N)); /* reminder: nil is black too */

                            /* Invariant:
                             * - either:
                             *   - N is nil (first iteration only), and
                             *     P->child(d) = nil, or:
                             *   - P is nil and non-nil N is tree root, or:
                             *   - N is an immediate child of P,
                             *     and P->child(d) = N
                             * - N is black
                             * - all paths that don't go thru N have prevailing black-height h.
                             * - paths through N have black-height h-1
                             */

                            if (!P) {
                                /* N is the root node, in which case all paths go through N,
                                 * so black-height is h-1
                                 */
                                *pp_root = N;
                                return;
                            }

                            other_d = other(d);
                            S = P->child(other_d);

                            /* S can't be nil:  since N is non-nil and black,
                             * it must have a non-nil sibling
                             */
                            assert(S);

                            C = S->child(d);
                            D = S->child(other_d);

                            if (log.enabled()) {
                                log("rebalance at parent P of curtailed subtree N",
                                    xtag("P", P),
                                    xtag("P.col", ccs(P->color() == C_Black ? "B" : "r")),
                                    xtag("P.key", P->key()));
                                log("with sibling S, nephews C,D", xtag("S", S),
                                    xtag("S.col", ccs(S->color() == C_Black ? "B" : "r")),
                                    xtag("C", C), xtag("D", D));
                            }

                            if (is_black(P) && is_black(S) && is_black(C) && is_black(D)) {
                                /* Case(1) */

                                log && log("P,S,C,D all black: mark S red + go up 1 level");

                                /* diagram with d=D_Left: flip left-to-right for d=D_Right
                                 *    =black
                                 *   *=red
                                 *   _=red or black
                                 *
                                 *     P
                                 *    / \
                                 *   N   S
                                 *      / \
                                 *     C   D
                                 *
                                 * relative to prevailing black-height h:
                                 * - N at h-1
                                 * - C at h
                                 * - D at h
                                 */

                                S->assign_color(C_Red);

                                /* now have:
                                 *
                                 *    G (=P')
                                 *    |
                                 *    P (=N')
                                 *   / \
                                 *  N   S*
                                 *     / \
                                 *    C   D
                                 *
                                 * relative to prevailing black-height h:
                                 * - N at h-1
                                 * - C at h-1
                                 * - D at h-1
                                 *
                                 * relabel to one level higher in tree
                                 */
                                N = P;
                                P = P->parent();
                                d = RbNode::child_direction(P, N);

                                continue;
                            } else {
                                break;
                            }
                        } /*loop looking for a red node*/

                        if (is_red(S)) {
                            /* Case(3) */

                            if (log.enabled()) {
                                log("case 3: S red, P,C,D black -> rotate at P to promote S");
                                log("case 3: + make P red instead of S");
                                log("case 3: with",
                                    xtag("P", P),
                                    xtag("P.col", ccs(P->color() == C_Black ? "B" : "r")),
                                    xtag("P.key", P->key()), xtag("S", S),
                                    xtag("S.col", ccs(S->color() == C_Black ? "B" : "r")),
                                    xtag("S.key", S->key()));
                            }

                            /* since S is red, {P,C,D} are all black
                             *
                             * diagram with d=D_Left: flip left-to-right for d=D_Right
                             *    =black
                             *   *=red
                             *   _=red or black
                             *
                             *     P
                             *    / \
                             *   N   S*
                             *      / \
                             *     C   D
                             *
                             * relative to prevailing black-height h:
                             * - N at h-1
                             * - C at h
                             * - D at h
                             */

                            assert(is_black(C));
                            assert(is_black(D));
                            assert(is_black(P));
                            assert(is_black(N));

                            RbTreeUtil::rotate(d, P, reduce_fn, pp_root);

                            /* after rotation d at P:
                             *
                             *       S*
                             *      / \
                             *     P   D
                             *    / \
                             *   N   C
                             *
                             * relative to prevailing black-height h:
                             * - N at h-1  (now goes thru red S)
                             * - C at H    (still goes through black P, red S)
                             * - D at h-1  (no longer goes thru black P)
                             */

                            P->assign_color(C_Red);
                            S->assign_color(C_Black);

                            /* after reversing colors of {P,S}:
                             *
                             *       S
                             *      / \
                             *     P*  D
                             *    / \
                             *   N   C (=S')
                             *
                             * relative to prevailing black-height h:
                             * - N at h-1 (now thru black S, red P instead of red S, black P)
                             * - C at h   (now thru black S, red P instead of red S, black P)
                             * - D at h   (now through black S instead of red S, black P)
                             */

                            /* now relabel for subsequent cases */
                            S = C;
                            C = S ? S->child(d) : nullptr;
                            D = S ? S->child(other_d) : nullptr;
                        }

                        assert(is_black(S));

                        if (is_red(P) && is_black(C) && is_black(D)) {
                            /* Case(4) */

                            if (log.enabled()) {
                                log("case 4: P red, N,S,C,D black -> recolor and finish");
                                log("case 4: with",
                                    xtag("P", P),
                                    xtag("P.col", ccs(P->color() == C_Black ? "B" : "r")),
                                    xtag("P.key", P->key()), xtag("S", S),
                                    xtag("S.col", ccs(S->color() == C_Black ? "B" : "r")),
                                    xtag("S.key", S->key()));
                            }

                            assert(is_black(N));

                            /* diagram with d=D_Left: flip left-to-right for d=D_Right*
                             *    =black
                             *   *=red
                             *   _=red or black
                             *
                             *     P*
                             *    / \
                             *   N   S
                             *      / \
                             *     C   D
                             *
                             * relative to prevailing black-height h:
                             * - N at h-1
                             * - C at h
                             * - D at h
                             */

                            P->assign_color(C_Black);
                            S->assign_color(C_Red);

                            /* after making P black, and S red (swapping colors of P,S):
                             *
                             *     P
                             *    / \
                             *   N   S*
                             *      / \
                             *     C   D
                             *
                             * relative to prevailing black-height h:
                             * - N at h
                             * - C at h
                             * - D at h
                             *
                             * and RB-shape is restored
                             */
                            return;
                        }

                        assert(is_black(S) && (is_black(P) || is_red(C) || is_red(D)));

                        if (is_red(C) && is_black(D)) {
                            log && log("case 5: C red, S,D black -> rotate at S");

                            /* diagram with d=D_Left;  flip left-to-right for d=D_Right
                             *
                             *    =black
                             *   *=red
                             *   _=red or black
                             *
                             *     P_
                             *    / \
                             *   N   S
                             *      / \
                             *     C*  D
                             *
                             * relative to prevailing black-height h:
                             * - N at h-1
                             * - C at h
                             * - D at h
                             */

                            RbTreeUtil::rotate(other_d, S, reduce_fn, pp_root);

                            assert(P->child(other_d) == C);

                            /* after other(d) rotation at S:
                             *
                             *     P_
                             *    / \
                             *   N   C*
                             *        \
                             *         S
                             *          \
                             *           D
                             *
                             * relative to prevailing black-height h:
                             * - N at h-1
                             * - C at h-1  (no longer goes thru black S)
                             * - S at h    (now goes thru red C)
                             * - D at h    (now goes thru red C)
                             */

                            C->assign_color(C_Black);
                            S->assign_color(C_Red);

                            /* after exchanging colors of C,S:
                             *
                             *     P_
                             *    / \
                             *   N   C (=S')
                             *        \
                             *         S* (=D')
                             *          \
                             *           D
                             *
                             * relative to prevailing black-height h:
                             * - N at h-1
                             * - C at h    (no longer goes thru black S, but now C black)
                             * - S at h    (no longer red, but now goes thru black C)
                             * - D at h    (now goes thru black C, red S instead of black S)
                             */

                            /* now relabel to match next and final case */
                            D = S;
                            S = C;
                            C = nullptr; /* won't be using C past this point */

                            assert(D);
                            assert(D->is_red());

                            /* fall through to next case */
                        }

                        if (is_red(D)) {
                            log && log("case 6: S black, D red -> rotate at P and finish");

                            /* diagram with d=D_Left;  flip left-to-right for d=D_Right
                             *
                             * Sibling is black,  and distant child is red
                             *
                             * if N=P->left_child():
                             *
                             *   *=red
                             *   _=red or black
                             *
                             *      P_
                             *     / \
                             *    N   S
                             *       / \
                             *      C_  D*
                             *
                             * relative to prevailing black-height h:
                             * - N   at h-1
                             * - S   (+also C,D) at h
                             */

                            RbTreeUtil::rotate(d, P, reduce_fn, pp_root);

                            /* after rotate at P toward d: *
                             *
                             *      S
                             *     / \
                             *    P_  D*
                             *   / \
                             *  N   C_
                             *
                             * Now,  relative to prevailing black-height h:
                             * - N at h+1  (paths to N now visit black S)
                             * - C at h    (paths to C still visit P,S)
                             * - D at: h   if P red,
                             *         h-1 if P black
                             *   (paths to D now skip P)
                             */

                            S->assign_color(P->color());
                            P->assign_color(C_Black);
                            D->assign_color(C_Black);

                            /* after recolor: S to old P color, P to black, D to black.
                             *
                             *       S_
                             *      / \
                             *     P   D
                             *    / \
                             *   N   C_
                             *
                             * Now, relative to prevailing black-height h:
                             * - N at h+1   (swapped P, S colors)
                             * - C at h     (paths to C still visit P,S,  swapped P,S colors)
                             * - D at: h    if S red  (was P red, S black, D red; now S red, D
                             * black) h    if S black (was P black, S black, D red; now S
                             * black, D black)
                             *
                             * RB-shape has been restored
                             */
                            return;
                        }
                    } /*remove_black_leaf*/

                /* remove node with key k from tree rooted at *pp_root.
                 * on exit *pp_root contains new tree root.
                 *
                 * Require:
                 * - pp_root is non-null.  (*pp_root can be null -> tree is empty)
                 * - *pp_root is in RB-shape
                 *
                 * return true if a node was removed;  false otherwise.
                 */
                template <typename NodeAllocator>
                static bool erase_aux(NodeAllocator & alloc,
                                      Key const & k,
                                      Reduce const & reduce_fn,
                                      bool debug_flag,
                                      RbNode ** pp_root) {
                    using xo::scope;
                    using xo::xtag;

                    scope log(XO_DEBUG(debug_flag));

                    RbNode * N = *pp_root;

                    log && log("enter", xtag("root", N));

                    /*
                     * here the triangle ascii art indicates a tree structure,
                     * of arbitrary size
                     *
                     *       o <- this
                     *      / \
                     *     o-P-o
                     *       |
                     *       N
                     *      / \
                     *     X   Y
                     *    / \
                     *   o---R
                     *      / .
                     *     W
                     */

                    /* N is the candidate target node we will be deleting */
                    N = RbTreeUtil::find_glb(N, k, true /*is_closed*/);

                    if (!N || (N->key() != k)) {
                        /* no node with .key = k present,  so cannot remove it */
                        return false;
                    }

                    if (log) {
                        log("got lower bound",
                            xtag("N", N),
                            xtag("N.key", N->key()));
                        RbTreeUtil::display_aux(D_Invalid, N, 0 /*depth*/, &log);
                    }

                    /* first step is to simplify problem so that we're removing
                     * a node with 0 or 1 children.
                     */

                    RbNode * P = N->parent();
                    RbNode * X = N->left_child();
                    RbNode * Y = N->right_child();

                    log && log(xtag("P", P),
                               xtag("X", X),
                               xtag("Y", Y));

                    if (X == nullptr) {
                        /* N has 0 or 1 children */
                        erase_1child_aux(alloc, N, reduce_fn, debug_flag, pp_root);
                    } else {
                        /* R->right_child() is nil by definition */

                        RbNode * R = RbTreeUtil::find_rightmost(X);
                        assert(R);
                        assert(R->right_child() == nullptr);

                        RbNode * W = R->left_child();

                        log && log(xtag("R", R), xtag("W", W));

                        if (P == nullptr) {
                            /* R will replace N -> becomes new root */
                            *pp_root = R;
                        }

                        if (R == N->left_child()) {
                            /*
                             * here the triangle ascii art indicates a tree structure,
                             * of arbitrary size
                             *
                             *       o <- this
                             *      / \
                             *     o-P-o
                             *       |
                             *       N
                             *      / \
                             *     R   Y
                             *    / .
                             *   W
                             */
                            if (P)
                                P->replace_child_reparent(N, R);

                            /*
                             * here the triangle ascii art indicates a tree structure,
                             * of arbitrary size
                             *
                             *       o <- this
                             *      / \
                             *     o-P-o
                             *       |      N
                             *       R     / \
                             *      / .   R   Y
                             *     W
                             */
                            R->assign_child_reparent(D_Left, N);
                            R->assign_child_reparent(D_Right, Y);
                            /*
                             * here the triangle ascii art indicates a tree structure,
                             * of arbitrary size
                             *
                             *       o <- this
                             *      / \
                             *     o-P-o
                             *       |
                             *       R
                             *      / \
                             *     N   Y
                             *    / \
                             *   R   Y
                             */
                            N->assign_child_reparent(D_Left, W);
                            N->assign_child_reparent(D_Right, nullptr);
                            /*
                             * here the triangle ascii art indicates a tree structure,
                             * of arbitrary size
                             *
                             *       o <- this
                             *      / \
                             *     o-P-o
                             *       |
                             *       R
                             *      / \
                             *     N   Y
                             *    / \
                             *   W   .
                             */

                            /* need also to swap other node attributes:
                             *   RbNode .color_ .size_ .reduced_
                             */
                            std::swap(R->color_, N->color_);
                            std::swap(N->size_, N->size_);

                            if (log) {
                                log("after swapping N,R locations:");
                                RbTreeUtil::display_aux(D_Invalid, R, 0 /*depth*/, &log);
                            }

                            erase_1child_aux(alloc, N, reduce_fn, debug_flag, pp_root);
                        } else {
                            /*
                             * here the triangle ascii art indicates a tree structure,
                             * of arbitrary size
                             *
                             *       o <- this
                             *      / \
                             *     o-P-o
                             *       |
                             *       N
                             *      / \
                             *     X   Y
                             *    / \
                             *   o---R
                             *      / .
                             *     W
                             */

                            /* will be swapping info in {R, N}:
                             * everything except RbNode.contents_.
                             * Annoying but necessary to have stable Node memory locations
                             */
                            RbNode::swap_locations(R, N, debug_flag);

                            /* swapping locations invalidates RbNode.reduced_.
                             * But correctness will be restored in erase_1child_aux()
                             */

                            /*
                             * here the triangle ascii art indicates a tree structure,
                             * of arbitrary size
                             *
                             *       o <- this
                             *      / \
                             *     o-P-o
                             *       |
                             *       R
                             *      / \
                             *     X   Y
                             *    / \
                             *   o---N
                             *      / .
                             *     W
                             */
                            erase_1child_aux(alloc, N, reduce_fn, debug_flag, pp_root);
                        }

#ifdef OBSOLETE
                        /* would be convenient to just make this assignment,
                         * but several disadvantages:
                         * 1. invalidates an iterator pointing to R
                         *    when nearby-in-key-space N gets deleted
                         * 2. gives up key constness
                         */
                        N->contents_ = R->contents_; N = R;
#endif
                    }

                    return true;
                } /*erase_aux*/

                template <typename NodeAllocator>
                static void erase_1child_aux(NodeAllocator & alloc,
                                             RbNode * N,
                                             Reduce const & reduce_fn,
                                             bool debug_flag,
                                             RbNode ** pp_root) {

                    scope log(XO_DEBUG(debug_flag));

                    using traits = std::allocator_traits<NodeAllocator>;

                    RbNode * P = N->parent();

                    /* N has 0 or 1 children
                     *
                     * Implications:
                     * 1. if N is red, it cannot have red children (by RB rules),
                     *    and it cannot have just 1 black child.
                     *    Therefore red N must have 0 children
                     *     -> can delete N without disturbing RB properties
                     * 2. if N is black:
                     *    2.1 if N has 1 child S,  then S must be red
                     *        (if S were black,  that would require N to have a 2nd child
                     *         to preserve equal black-height for all paths)
                     *     -> replace N with S, repainting S black,  in place of
                     *        to-be-reclaimed N
                     *    1.2 if N is black with 0 children,  need to rebalance
                     */

                    if (N->is_red()) {
                        if (N->is_leaf()) {
                            /* replace pointer to N with nil in N's parent. */

                            if (P) {
                                P->replace_child_reparent(N, nullptr);

                                log && log("fixup_ancestor_size starting at P");
                                RbTreeUtil::fixup_ancestor_size(reduce_fn, P, debug_flag);
                            } else {
                                /* N was sole root node;  tree will be empty after removing it */
                                log && log("deleting last node -> *pp_root=nullptr");
                                *pp_root = nullptr;
                            }

                            log && log("delete red root node", xtag("addr", N));
                            traits::deallocate(alloc, N, 1);
                        } else {
                            assert(false);

                            /* control can't come here for RB-tree,
                             * because a red node can't have red children,  or just one black
                             * child.
                             */
                        }
                    } else /*N->is_black()*/ {
                        RbNode *R = N->left_child();

                        if (!R)
                            R = N->right_child();

                        if (R) {
                            /* if a black node has one child,  that child cannot be black */
                            assert(R->is_red());

                            /* replace N with R in N's parent,
                             * + make R black to preserve black-height
                             */
                            R->assign_color(C_Black);

                            if (P) {
                                P->replace_child_reparent(N, R);
                                log && log("fixup_ancestor_size starting at P");
                                RbTreeUtil::fixup_ancestor_size(reduce_fn, P, debug_flag);
                            } else {
                                /* N was root node */
                                RbNode::replace_root_reparent(R, pp_root);
                            }

                            log && log("delete node", xtag("addr", N));
                            traits::deallocate(alloc, N, 1);
                        } else {
                            /* N is black with no children,
                             * may need rebalance here
                             */

                            if (P) {
                                RbTreeUtil::remove_black_leaf(alloc, N, reduce_fn, debug_flag, pp_root);
                            } else {
                                /* N was root node */
                                *pp_root = nullptr;

                                log && log("delete black root node", xtag("addr", N));
                                traits::deallocate(alloc, N, 1);
                            }
                        }
                    }
                } /*erase_1child_aux*/

                /* verify that subtree at N is in RB-shape.
                 * will cover subset of RedBlackTree class invariants:
                 *
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
                 * RB8. inorder traversal visits all the keys in subtree
                 *
                 * returns the #of nodes in subtree rooted at N.
                 */
                static size_t verify_subtree_ok(Reduce const & reduce_fn,
                                                RbNode const * N,
                                                int32_t * p_black_height)
                    {
                        using xo::scope;
                        using xo::xtag;
                        using xo::print::ccs;

                        constexpr char const *c_self = "RbTreeUtil::verify_subtree_ok";

                        // scope lscope(c_self);

                        /* counts #of nodes in subtree rooted at N */
                        size_t i_node = 0;
                        Key const *last_key = nullptr;
                        /* inorder node index when establishing black_height */
                        size_t i_black_height = 0;
                        /* establish on first leaf node encountered */
                        uint32_t black_height = 0;

                        auto verify_fn = [c_self,
                                          &reduce_fn,
                                          &i_node,
                                          &last_key,
                                          &i_black_height,
                                          &black_height] (RbNode const *x,
                                                          uint32_t bd)
                            {
                                /* RB2. if c=x->child(d), then c->parent()=x */

                                if (x->left_child()) {
                                    XO_EXPECT(x == x->left_child()->parent(),
                                              tostr(c_self, (": expect symmetric child/parent pointers"),
                                                    xtag("i", i_node), xtag("node[i]", x),
                                                    xtag("key[i]", x->key()),
                                                    xtag("child", x->left_child()),
                                                    xtag("child.key", x->left_child()->key()),
                                                    xtag("child.parent", x->left_child()->parent_)));
                                }

                                if (x->right_child()) {
                                    XO_EXPECT(x == x->right_child()->parent(),
                                              tostr(c_self, ": expect symmetric child/parent pointers",
                                                    xtag("i", i_node),
                                                    xtag("node[i]", x),
                                                    xtag("key[i]", x->key()),
                                                    xtag("child", x->right_child()),
                                                    xtag("child.key", x->right_child()->key()),
                                                    xtag("child.parent", x->right_child()->parent_)));
                                }

                                /* RB3. all nodes have the same black-height */

                                if (x->is_leaf()) {
                                    if (black_height == 0) {
                                        black_height = bd;
                                    } else {
                                        XO_EXPECT(black_height == bd,
                                                  tostr(c_self,
                                                        ": expect all RB-tree nodes to have the same "
                                                        "black-height",
                                                        xtag("i1", i_black_height), xtag("i2", i_node),
                                                        xtag("blackheight(i1)", black_height),
                                                        xtag("blackheight(i2)", bd)));
                                    }
                                }

                                /* RB4. a red node may not have a red parent
                                 *      (conversely,  a red node may not have a red child)
                                 */

                                RbNode *red_child =
                                    ((x->left_child() && x->left_child()->is_red())
                                     ? x->left_child()
                                     : ((x->right_child() && x->right_child()->is_red())
                                        ? x->right_child()
                                        : nullptr));

                                XO_EXPECT(
                                    x->is_red_violation() == false,
                                    tostr(c_self,
                                          ccs(": expect RB-shape tree to have no red violations but "
                                              "red y is child of red x"),
                                          xtag("i", i_node), xtag("x.addr", x),
                                          xtag("x.col", ccs((x->color_ == C_Black) ? "B" : "r")),
                                          xtag("x.key", x->key()),
                                          xtag("y.addr", red_child),
                                          xtag("y.col", ccs((red_child->color_ == C_Black) ? "B" : "r")),
                                          xtag("y.key", red_child->key())));

                                /* RB5.  inorder traversal visits nodes in strictly increasing key order */

                                if (last_key) {
                                    XO_EXPECT((*last_key) < x->key(),
                                              tostr(c_self,
                                                    ": expect inorder traversal to visit keys"
                                                    " in strictly increasing order",
                                                    xtag("i", i_node), xtag("key[i-1]", *last_key),
                                                    xtag("key[i]", x->key())));
                                }

                                last_key = &(x->key());

                                /* RB6. Node::size reports the size of the subtree reachable from that
                                 *      node by child pointers.
                                 */
                                XO_EXPECT(x->size() == (tree_size(x->left_child())
                                                        + 1
                                                        + tree_size(x->right_child())),
                                          tostr(c_self,
                                                ": expect Node::size to be 1 + sum of childrens' size",
                                                xtag("i", i_node),
                                                xtag("key[i]", x->key()),
                                                xtag("left.size", tree_size(x->left_child())),
                                                xtag("right.size", tree_size(x->right_child()))));

                                /* RB7. Node::reduced reports the value of
                                 *       f(f(L, Node::value), R)
                                 *      where: L is reduced-value for left child,
                                 *             R is reduced-value for right child
                                 */
                                auto reduced_pair
                                    = RbNode::reduced_pair(reduce_fn, x);

                                XO_EXPECT(reduce_fn.is_equal
                                          (x->reduced1(), reduced_pair.first),
                                          tostr(c_self,
                                                ": expect Node::reduced to be reduce_fn"
                                                " applied to (.L, .value)",
                                                xtag("node.reduced1", x->reduced1()),
                                                xtag("reduced_pair.first", reduced_pair.first)));

                                XO_EXPECT(reduce_fn.is_equal
                                          (x->reduced2(), reduced_pair.second),
                                          tostr(c_self,
                                                ": expect Node::reduced to be reduce_fn"
                                                " applied to (.L, .value, .R)",
                                                xtag("node.reduced2", x->reduced2()),
                                                xtag("reduce2_expr", reduced_pair.second)));

                                ++i_node;
                            };

                        RbTreeUtil::inorder_node_visitor(N, 0 /*d*/, verify_fn);

                        if (p_black_height)
                            *p_black_height = black_height;

                        /* RB8. inorder traversal visits all the nodes */
                        std::size_t subtree_z = N ? N->size() : 0ul;

                        XO_EXPECT(i_node == subtree_z,
                                  tostr(c_self, ": expect visit count = node.size",
                                        xtag("visit_count", i_node),
                                        xtag("node.size", 0)));

                        return i_node;
                    } /*verify_subtree_ok*/

                /** display tree structure,  1 line per node.
                 *  indent by node depth @p d
                 **/
                static void display_aux(Direction side, RbNode const *N, uint32_t d,
                                        xo::scope *p_scope) {
                    using xo::pad;
                    using xo::xtag;
                    using xo::print::ccs;

                    if (N) {
                        p_scope->log(pad(d),
                                     xtag("addr", N),
                                     xtag("par", N->parent()),
                                     xtag("side", ccs((side == D_Left)    ? "L"
                                                      : (side == D_Right) ? "R"
                                                      : "root")),
                                     xtag("col", ccs(N->is_black() ? "B" : "r")),
                                     xtag("key", N->key()),
                                     xtag("value", N->value()),
                                     xtag("wt", N->size()),
                                     xtag("reduced1", N->reduced1()),
                                     xtag("reduced2", N->reduced2()));
                        display_aux(D_Left, N->left_child(), d + 1, p_scope);
                        display_aux(D_Right, N->right_child(), d + 1, p_scope);
                    }
                } /*display_aux*/

                /**
                 *  indent by node depth @p d
                 **/
                static void display(RbNode const *N, uint32_t d) {
                    using xo::scope;

                    scope log(XO_DEBUG(true /*debug_flag*/));

                    display_aux(D_Invalid, N, d, &log);
                } /*display*/
            }; /*RbTreeUtil*/
    } /*namespace detail*/

    } /*namespace tree*/
} /*namespace xo*/

/* end RbTreeUtil.hpp */
