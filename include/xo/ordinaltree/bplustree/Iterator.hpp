/* @file Iterator.hpp */

#pragma once

#include "IteratorUtil.hpp"
#include "LeafNode.hpp"
#include "indentlog/print/tostr.hpp"

namespace xo {
    namespace tree {
        namespace detail {
            /* TODO: move to tree/IteratorUtil.hpp */

            /* placeholder - specialize on isConst */
            template <typename Key,
                      typename Value,
                      typename Properties,
                      bool isConst>
            struct NodeTypeTraits { using LeafNodeType = void; };

            /* non-const node pointer */
            template <typename Key,
                      typename Value,
                      typename Properties>
            struct NodeTypeTraits<Key, Value, Properties, false /*!isConst*/> {
                using NativeLeafNodeType = LeafNode<Key, Value, Properties>;
                using LeafNodeType = NativeLeafNodeType;
                using NativeContentsType = typename LeafNodeType::ContentsType;
                using LeafNodePtrType = LeafNodeType *;
            };

            /* const node pointer */
            template <typename Key,
                      typename Value,
                      typename Properties>
            struct NodeTypeTraits<Key, Value, Properties, true /*isConst*/> {
                using NativeLeafNodeType = LeafNode<Key, Value, Properties>;
                using LeafNodeType = NativeLeafNodeType const;
                using NativeContentsType = typename LeafNodeType::ContentsType const;
                using LeafNodePtrType = LeafNodeType const *;
            };

            /* shared between const and non-const b+ tree iterators
             *
             *         +------------+
             *         |IteratorBase|
             *         | .dirn      |
             *         | .location  |
             *         | .leafnode  |
             *         | .ix        |
             *         +------------+
             *               ^
             *               |   isa     +-------------+
             *               +-----------|ConstIterator|
             *               |           | .operator++ |
             *               |           | .operator-- |
             *               |           +-------------+
             *               |
             *               |   isa     +--------+
             *               +-----------|Iterator|
             *                           +--------+
             */
            template <typename Key,
                      typename Value,
                      typename Properties,
                      bool isConst>
            class IteratorBase {
            public:
                using Traits = NodeTypeTraits<Key, Value, Properties, isConst>;
                using BpLeafNodePtrType = typename Traits::LeafNodePtrType;
                using BpLeafNodeItemType = typename Traits::LeafNodeType::LeafNodeItemType;
                using NativeContentsType = typename Traits::NativeContentsType;

            protected:
                IteratorBase() = default;
                IteratorBase(IteratorDirection dirn, IteratorLocation loc, BpLeafNodePtrType leaf, std::size_t ix)
                    : dirn_{dirn}, location_{loc}, leafnode_{leaf}, ix_{ix} {}
                IteratorBase(IteratorBase const &) = default;

                static IteratorBase prebegin_aux(BpLeafNodePtrType node) {
                    return IteratorBase(ID_Forward, IL_BeforeBegin, node, 0 /*ix*/);
                }

                static IteratorBase begin_aux(BpLeafNodePtrType node) {
                    return IteratorBase(ID_Forward,
                                        (node ? IL_Regular : IL_AfterEnd),
                                        node,
                                        0 /*ix*/);
                }

                static IteratorBase end_aux(BpLeafNodePtrType node) {
                    return IteratorBase(ID_Forward,
                                        IL_AfterEnd,
                                        node,
                                        0 /*ix*/);
                }

                static IteratorBase rprebegin_aux(BpLeafNodePtrType node) {
                    return IteratorBase(ID_Reverse,
                                        IL_AfterEnd,
                                        node,
                                        0 /*ix*/);
                }

                static IteratorBase rbegin_aux(BpLeafNodePtrType node) {
                    return IteratorBase(ID_Reverse,
                                        (node ? IL_Regular : IL_BeforeBegin),
                                        node,
                                        (node ? node->n_elt() - 1: 0));
                }

                static IteratorBase rend_aux(BpLeafNodePtrType node) {
                    return IteratorBase(ID_Reverse,
                                        IL_BeforeBegin,
                                        node,
                                        0 /*ix*/);
                }

            public:
                IteratorLocation location() const { return location_; }
                BpLeafNodePtrType node() const { return leafnode_; }
                BpLeafNodeItemType const * item_addr() const { return &(leafnode_->lookup_elt(this->ix_)); }

                NativeContentsType const & operator*() const {
                    this->check_regular();
                    return this->leafnode_->lookup_elt(this->ix_).kv_pair();
                } /*operator**/

                NativeContentsType const * operator->() const {
                    return &(this->operator*());
                } /*operator->*/

                bool is_sentinel() const { return (this->location_ != IL_Regular); }
                bool is_dereferenceable() const { return !this->is_sentinel(); }

                operator bool() const { return this->is_deferenceable(); }

                bool operator==(IteratorBase const & x) const {
                    return  (this->location_ == x.location_) && (this->leafnode_ == x.leafnode_) && (this->ix_ == x.ix_);
                }

                bool operator!=(IteratorBase const & x) const {
                    return (this->location_ != x.location_) || (this->leafnode_ != x.leafnode_) || (this->ix_ != x.ix_);
                }

                void print(std::ostream & os) const {
                    using xo::xtag;

                    os << "<bptree-iterator"
                       << xtag("dirn", dirn_)
                       << xtag("loc", location_)
                       << xtag("leaf", leafnode_)
                       << xtag("ix", ix_)
                       << ">";
                } /*print*/

                /* pre-increment */
                IteratorBase & operator++() {
                    return ((this->dirn_ == ID_Forward)
                            ? this->next_step()
                            : this->prev_step());
                } /*operator++*/

                /* pre-decrement */
                IteratorBase & operator--() {
                    return ((this->dirn_ == ID_Forward)
                            ? this->prev_step()
                            : this->next_step());
                } /*operator--*/

            private:
                IteratorBase & next_step() {
                    switch(this->location_) {
                    case IL_BeforeBegin:
                        /* .leafnode is leftmost node in tree */
                        this->location_ = IL_Regular;
                        break;
                    case IL_Regular:
                        {
                            /* #of elts in node,  not #of elts in tree! */
                            std::size_t n_elt = this->leafnode_->n_elt();

                            if (this->ix_ + 1 < n_elt) {
                                ++(this->ix_);
                            } else if (this->leafnode_->next_leafnode()) {
                                this->leafnode_ = this->leafnode_->next_leafnode();
                                this->ix_ = 0;
                            } else {
                                /* preserve .leafnode:
                                 * (a) for == comparison w/ .end() iterator
                                 * (b) so we can iterate backwards from end position
                                 */
                                //this->leafnode_ = this->leafnode_->next_leafnode();
                                this->location_ = IL_AfterEnd;
                                this->ix_ = 0;
                            }
                        }
                        break;
                    case IL_AfterEnd:
                        break;
                    }

                    return *this;
                } /*next_step*/

                IteratorBase & prev_step() {
                    switch(this->location_) {
                    case IL_BeforeBegin:
                        break;
                    case IL_Regular:
                        if (this->ix_ > 0) {
                            --(this->ix_);
                        } else if (this->leafnode_->prev_leafnode()) {
                            this->leafnode_ = this->leafnode_->prev_leafnode();
                            this->ix_ = this->leafnode_->n_elt() - 1;
                        } else /* .ix == 0 && .leafnode.prev_leafnode == nullptr */ {
                            /* preserve .leafnode:
                             * (a) for == comparison w/ .prebegin() iterator
                             * (b) so iterator is reversible;  can iterate forwards from prebegin position
                             */
                            this->location_ = IL_BeforeBegin;
                        }
                        break;
                    case IL_AfterEnd:
                        /* .leafnode is rightmost node in tree */
                        this->location_ = IL_Regular;
                        this->ix_ = this->leafnode_->n_elt() - 1;
                        break;
                    }

                    return *this;
                } /*prev_step*/

            private:
                void check_regular() const {
                    using xo::tostr;
                    using xo::xtag;

                    if (this->location_ != IL_Regular) {
                        throw std::runtime_error(tostr("bplustree iterator: cannot deref iterator"
                                                       " in sentinel state",
                                                       xtag("loc", this->location_)));
                    }
                } /*check_regular*/

            private:
                /* ID_Forward     forward iterator
                 * ID_Reverse     reverse iterator
                 */
                IteratorDirection dirn_ = ID_Forward;
                /* IL_BeforeBegin | IL_Regular | IL_AfterEnd
                 *
                 *                    operator++                 operator++
                 *  IL_BeforeBegin  --------------> IL_Regular  --------------> IL_AfterEnd
                 *                              /->             -\
                 *                              |                |
                 *                              \----------------/
                 *                                 operator++
                 *
                 *                    operator--                 operator--
                 *  IL_BeforeBegin  <-------------  IL_Regular <-------------- IL_AfterEnd
                 *                             /--             <-\
                 *                             |                 |
                 *                             \-----------------/
                 *                                  operator--
                 *
                 *
                 */
                IteratorLocation location_ = IL_AfterEnd;
                /* .location        .leafnode
                 * IL_BeforeBegin   BplusTree.leafnode_begin   (leftmost leaf node)
                 * IL_Regular       any leaf node reachable from BplusTree.leafnode_begin
                 *                  (or equivalently from BplusTree.leafnode_end)
                 * IL_AfterEnd      BplusTree.leafnode_end     (rightmost leaf node)
                 */
                BpLeafNodePtrType leafnode_ = nullptr;
                /* index position within .leafnode;
                 * 0 when .location is IL_BeforeBegin | IL_AfterEnd
                 */
                std::size_t ix_ = 0;
            }; /*IteratorBase*/

            template <typename Key,
                      typename Value,
                      typename Properties>
            class ConstIterator : public IteratorBase<Key, Value, Properties, true /*isConst*/> {
            public:
                using iterator_concept = std::bidirectional_iterator_tag;

                using BpIteratorBase = IteratorBase<Key, Value, Properties, true /*isConst*/>;
                using BpLeafNodePtrType = typename BpIteratorBase::BpLeafNodePtrType;

            public:
                ConstIterator() = default;
                ConstIterator(IteratorDirection dirn, IteratorLocation loc, BpLeafNodePtrType leaf, std::size_t ix)
                    : IteratorBase<Key, Value, Properties, true /*isConst*/>(dirn, loc, leaf, ix) {}
                ConstIterator(ConstIterator const & x) = default;
                ConstIterator(BpIteratorBase const & x) : BpIteratorBase(x) {}
                ConstIterator(BpIteratorBase && x) : BpIteratorBase{std::move(x)} {}

                static ConstIterator prebegin_aux(BpLeafNodePtrType leaf) { return BpIteratorBase::prebegin_aux(leaf); }
                static ConstIterator begin_aux(BpLeafNodePtrType leaf) { return BpIteratorBase::begin_aux(leaf); }
                static ConstIterator end_aux(BpLeafNodePtrType leaf) { return BpIteratorBase::end_aux(leaf); }

                static ConstIterator rprebegin_aux(BpLeafNodePtrType leaf) { return BpIteratorBase::rprebegin_aux(leaf); }
                static ConstIterator rbegin_aux(BpLeafNodePtrType leaf) { return BpIteratorBase::rbegin_aux(leaf); }
                static ConstIterator rend_aux(BpLeafNodePtrType leaf) { return BpIteratorBase::rend_aux(leaf); }

                /* pre-increment */
                ConstIterator & operator++() {
                    BpIteratorBase::operator++();
                    return *this;
                } /*operator++*/

                /* post-increment */
                ConstIterator operator++(int) {
                    ConstIterator retval = *this;

                    ++(*this);

                    return retval;
                } /*operator++*/

                /* pre-decrement */
                ConstIterator & operator--() {
                    BpIteratorBase::operator--();
                    return *this;
                } /*operator--*/

                /* post-decrement */
                ConstIterator operator--(int) {
                    ConstIterator retval = *this;

                    --(*this);

                    return retval;
                } /*operator--*/
            }; /*ConstIterator*/
        } /*namespace detail*/

        template <typename Key,
                  typename Value,
                  typename Properties,
                  bool IsConst>
        inline std::ostream &
        operator<<(std::ostream & os,
                   detail::IteratorBase<Key, Value, Properties, IsConst> const & iter)
        {
            iter.print(os);
            return os;
        } /*operator<<*/
    } /*namespace tree*/
} /*namespace xo*/

/* end Iterator.hpp */
