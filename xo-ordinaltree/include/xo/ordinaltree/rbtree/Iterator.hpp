/** @file Iterator.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "Node.hpp"

namespace xo {
    namespace tree {
        namespace detail {
            /* tragically, we can't partially specialize an alias template.
             * however we /can/ partially specialize a struct that nests a typealias.
             */
            template <typename Key,
                      typename Value,
                      typename Reduce,
                      bool IsConst>
            struct NodeTypeTraits { using NodeType = void; };

            template <typename Key,
                      typename Value,
                      typename Reduce>
            struct NodeTypeTraits<Key, Value, Reduce, false> {
                using NativeNodeType = Node<Key, Value, Reduce>;
                using NodeType = NativeNodeType;
                using ContentsType = typename NodeType::value_type;
                using NodePtrType = NodeType *;
            };

            template <typename Key,
                      typename Value,
                      typename Reduce>
            struct NodeTypeTraits<Key, Value, Reduce, true> {
                using NativeNodeType = Node<Key, Value, Reduce>;
                using NodeType = NativeNodeType const;
                using ContentsType = typename NodeType::value_type const;
                using NodePtrType = NodeType const *;
            };

            /* xo::tree::detail::IteratorBase
             *
             * shared between const & and non-const red-black-tree iterators.
             *
             * editor bait: BaseIterator
             */
            template <typename Key,
                      typename Value,
                      typename Reduce,
                      bool IsConst>
            class IteratorBase {
            public:
                using RbUtil = RbTreeUtil<Key, Value, Reduce>;
                using RbNode = Node<Key, Value, Reduce>;
                using Traits = NodeTypeTraits<Key, Value, Reduce, IsConst>;
                using ReducedValue = typename Reduce::value_type;
                using RbNativeNodeType = typename Traits::NativeNodeType;
                using RbNodePtrType = typename Traits::NodePtrType;
                using RbContentsType = typename Traits::ContentsType;

            protected:
                IteratorBase() = default;
                IteratorBase(IteratorDirection dirn, IteratorLocation loc, RbNodePtrType node)
                    : dirn_{dirn}, location_{loc}, node_{node} {}
                IteratorBase(IteratorBase const & x) = default;

                static IteratorBase prebegin_aux(RbNodePtrType node) {
                    return IteratorBase(ID_Forward, IL_BeforeBegin, node);
                } /*prebegin_aux*/

                static IteratorBase begin_aux(RbNodePtrType node) {
                    return IteratorBase(ID_Forward, node ? IL_Regular : IL_AfterEnd, node);
                } /*begin_aux*/

                static IteratorBase end_aux(RbNodePtrType node) {
                    return IteratorBase(ID_Forward, IL_AfterEnd, node);
                } /*end_aux*/

                static IteratorBase rprebegin_aux(RbNodePtrType node) {
                    return IteratorBase(ID_Reverse, IL_AfterEnd, node);
                } /*rprebegin_aux*/

                static IteratorBase rbegin_aux(RbNodePtrType node) {
                    return IteratorBase(ID_Reverse,
                                        (node ? IL_Regular : IL_BeforeBegin),
                                        node);
                } /*rbegin_aux*/

                static IteratorBase rend_aux(RbNodePtrType node) {
                    return IteratorBase(ID_Reverse,
                                        IL_BeforeBegin,
                                        node);
                } /*rend_aux*/

            public:
                IteratorLocation location() const { return location_; }
                RbNodePtrType node() const { return node_; }

                ReducedValue const & reduced() const { return node_->reduced(); }

                RbContentsType & operator*() const {
                    this->check_regular();
                    return this->node_->contents();
                } /*operator**/

                RbContentsType * operator->() const {
                    return &(this->operator*());
                }

                /* true for "just before beginning" and "just after the end" states.
                 * false otherwise
                 */
                bool is_sentinel() const { return (this->location_ != IL_Regular); }
                /* true unless iterator is in a sentinel state */
                bool is_dereferenceable() const { return !this->is_sentinel(); }

                /* deferenceable iterators are truth-y;
                 * sentinel iterators are false-y
                 */
                operator bool() const { return this->is_dereferenceable(); }

                bool operator==(IteratorBase const & x) const {
                    return (this->location_ == x.location_) && (this->node_ == x.node_);
                } /*operator==*/

                bool operator!=(IteratorBase const & x) const {
                    return (this->location_ != x.location_) || (this->node_ != x.node_);
                } /*operator!=*/

                void print(std::ostream & os) const {
                    using xo::xtag;

                    os << "<rbtree-iterator"
                       << xtag("dirn", dirn_)
                       << xtag("loc", location_)
                       << xtag("node", node_)
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

            protected:
                void check_regular() const {
                    using xo::tostr;

                    if(this->location_ != IL_Regular)
                        throw std::runtime_error(tostr("rbtree iterator: cannot deref iterator"
                                                       " in non-regular state"));
                } /*check_regular*/

            private:
                IteratorBase & next_step() {
                    switch(this->location_) {
                    case IL_BeforeBegin:
                        /* .node is first node in tree */
                        this->location_ = IL_Regular;
                        break;
                    case IL_Regular:
                        {
                            RbNodePtrType next_node
                                = RbUtil::next_inorder_node(const_cast<RbNativeNodeType *>(this->node_));

                            if(next_node) {
                                this->node_ = next_node;
                            } else {
                                this->location_ = IL_AfterEnd;
                            }
                        }
                        break;
                    case IL_AfterEnd:
                        break;
                    } /*operator++*/

                    return *this;
                } /*next_step*/

                IteratorBase & prev_step() {
                    switch(this->location_) {
                    case IL_BeforeBegin:
                        break;
                    case IL_Regular:
                        {
                            RbNode * prev_node = RbUtil::prev_inorder_node(const_cast<RbNativeNodeType *>(this->node_));

                            if(prev_node) {
                                this->node_ = prev_node;
                            } else {
                                this->location_ = IL_BeforeBegin;
                            }
                        }
                        break;
                    case IL_AfterEnd:
                        /* .node is already last node in tree */
                        this->location_ = IL_Regular;
                        break;
                    }

                    return *this;
                } /*prev_step*/

            protected:
                /* ID_Forward, ID_Reverse */
                IteratorDirection dirn_ = ID_Forward;
                /* IL_BeforeBegin, IL_Regular, IL_AfterEnd */
                IteratorLocation location_ = IL_AfterEnd;
                /* location = IL_BeforeBegin: .node is leftmost node in tree
                 * location = IL_Regular:     .node is some node in tree,
                 *                            iterator refers to that node.
                 * location = IL_AfterEnd:    .node is rightmost node in tree
                 */
                RbNodePtrType node_ = nullptr;
            }; /*IteratorBase*/

            /* xo::tree::detail::Iterator
             *
             * inorder iterator over nodes in a red-black tree.
             * invalidated on insert or remove operations on the parent tree.
             *
             * satisfies the std::bidirectional_iterator concept
             */
            template <typename Key,
                      typename Value,
                      typename Reduce>
            class Iterator : public IteratorBase<Key, Value, Reduce, false /*!IsConst*/> {
            public:
                using iterator_concept = std::bidirectional_iterator_tag;

                using RbIteratorBase = IteratorBase<Key, Value, Reduce, false /*!IsConst*/>;
                using RbNode = typename RbIteratorBase::RbNode;
                using RbUtil = typename RbIteratorBase::RbUtil;
                using ReducedValue = typename Reduce::value_type;

            public:
                Iterator() = default;
                Iterator(IteratorDirection dirn, IteratorLocation loc, RbNode * n)
                    : RbIteratorBase(dirn, loc, n) {}
                Iterator(Iterator const & x) = default;
                Iterator(RbIteratorBase const & x) : RbIteratorBase(x) {}
                Iterator(RbIteratorBase && x) : RbIteratorBase(std::move(x)) {}

                static Iterator begin_aux(RbNode const * n) { return RbIteratorBase::begin_aux(n); }

                static Iterator end_aux(RbNode const * n) { return RbIteratorBase::end_aux(n); }

                static Iterator rbegin_aux(RbNode const * n) { return RbIteratorBase::rbegin_aux(n); }
                static Iterator rend_aux(RbNode const * n) { return RbIteratorBase::rend_aux(n); }

                /* pre-increment */
                Iterator & operator++() {
                    RbIteratorBase::operator++();
                    return *this;
                } /*operator++*/

                /* post-increment */
                Iterator operator++(int) {
                    Iterator retval = *this;

                    ++(*this);

                    return retval;
                } /*operator++(int)*/

                /* pre-decrement */
                Iterator & operator--() {
                    RbIteratorBase::operator--();
                    return *this;
                } /*operator--*/

                /* post-decrement */
                Iterator operator--(int) {
                    Iterator retval = *this;

                    --(*this);

                    return retval;
                } /*operator--(int)*/
            }; /*Iterator*/

            /* xo::tree::detail::ConstIterator
             *
             * inorder iterator over nodes in a red-black tree.
             * invalidated on insert or remove operations on the parent tree.
             *
             * satisfies the std::bidirectional_iterator concept
             */
            template <typename Key,
                      typename Value,
                      typename Reduce>
            class ConstIterator : public IteratorBase<Key, Value, Reduce, true /*IsConst*/> {
            public:
                using iterator_concept = std::bidirectional_iterator_tag;

                using RbIteratorBase = IteratorBase<Key, Value, Reduce, true /*IsConst*/>;
                using RbNode = typename RbIteratorBase::RbNode;
                using RbUtil = typename RbIteratorBase::RbUtil;
                using ReducedValue = typename Reduce::value_type;

            public:
                ConstIterator() = default;
                ConstIterator(IteratorDirection dirn, IteratorLocation loc, RbNode const * node)
                    : RbIteratorBase(dirn, loc, node) {}
                ConstIterator(ConstIterator const & x) = default;
                ConstIterator(RbIteratorBase const & x) : RbIteratorBase(x) {}
                ConstIterator(RbIteratorBase && x) : RbIteratorBase(std::move(x)) {}

                static ConstIterator prebegin_aux(RbNode const * n) { return RbIteratorBase::prebegin_aux(n); }
                static ConstIterator begin_aux(RbNode const * n) { return RbIteratorBase::begin_aux(n); }
                static ConstIterator end_aux(RbNode const * n) { return RbIteratorBase::end_aux(n); }

                static ConstIterator rprebegin_aux(RbNode const * n) { return RbIteratorBase::rprebegin_aux(n); }
                static ConstIterator rbegin_aux(RbNode const * n) { return RbIteratorBase::rbegin_aux(n); }
                static ConstIterator rend_aux(RbNode const * n) { return RbIteratorBase::rend_aux(n); }

                /* pre-increment */
                ConstIterator & operator++() {
                    RbIteratorBase::operator++();
                    return *this;
                } /*operator++*/

                /* post-increment */
                ConstIterator operator++(int) {
                    ConstIterator retval = *this;

                    ++(*this);

                    return retval;
                } /*operator++(int)*/

                /* pre-decrement */
                ConstIterator & operator--() {
                    RbIteratorBase::operator--();
                    return *this;
                } /*operator--*/

                /* post-decrement */
                ConstIterator operator--(int) {
                    ConstIterator retval = *this;

                    --(*this);

                    return retval;
                } /*operator--(int)*/
            }; /*ConstIterator*/
        } /*namespace detail*/
    } /*namespace tree*/
} /*namespace xo*/

/* end Iterator.hpp */
