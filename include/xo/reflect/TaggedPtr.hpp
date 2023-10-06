/* @file TaggedPtr.hpp */

#pragma once

#include "TypeDescr.hpp"
#include <unordered_set>

namespace xo {
    namespace reflect {
        class TaggedRcptr; /* see [reflect/TaggedRcptr.hpp] */

        class TaggedPtr {
        public:
            TaggedPtr(TypeDescr td, void * x) : td_{td}, address_{x} {}

            static TaggedPtr universal_null() { return TaggedPtr(nullptr, nullptr); }

            /* would be clean to put make() here;
             * however it leads to cyclic #include paths,
             * so put it elsewhere
             */
#ifdef NOT_USING
            template<typename T>
            static TaggedPtr make(T * x) { return TaggedPtr(Reflect::require<T>(), x); }
#endif

            /* visit an object tree.  calls preorder_visit_fn() on tp,
             * and all objects reachable directly-or-indirectly from tp.
             * will call preorder_visit_fn() multiple times if there are multiple paths
             * to a node.
             *
             * require: no cycles in object graph -- undefined behavior if a cycle is present
             */
            template<typename Fn>
            static void visit_tree_preorder(TaggedPtr tp, Fn && preorder_visit_fn) {
                using std::uint32_t;

                preorder_visit_fn(tp);

                for(uint32_t i = 0, n = tp.n_child(); i < n; ++i) {
                    visit_tree_preorder(tp.get_child(i), preorder_visit_fn);
                }
            } /*visit_tree_preorder*/

            /* visit object graph.  calls preorder_visit_fn() on tp in depth-first
             * order.  detects and silently prunes duplicate/cyclic references.
             */
            template<typename Fn>
            static void visit_graph(TaggedPtr tp, Fn && visit_fn) {
                std::unordered_set<void *> visited_set;

                visit_graph_aux(tp, visit_fn, &visited_set);
            } /*visit_graph*/

            TypeDescr td() const { return td_; }
            void * address() const { return address_; }

            void assign_td(TypeDescr x) { td_ = x; }
            void assign_address(void * x) { address_ = x; }

            bool is_universal_null() const { return (td_ == nullptr) && (address_ == nullptr); }
            bool is_vector() const { return td_ && td_->is_vector(); }
            bool is_struct() const { return td_ && td_->is_struct(); }


            /* returns pointer-to-T,  if in fact this tagged pointer is understood
             * to refer to a T-instance;  otherwise nullptr
             */
            template<typename T>
            T * recover_native() const { return this->td_->recover_native<T>(this->address_); }

            uint32_t n_child() const {
                return this->td_->n_child(this->address_);
            } /*n_child*/

            TaggedPtr get_child(uint32_t i) const {
                return this->td_->child_tp(i, this->address_);
            } /*get_child*/

            /* require:
             * - .is_struct() is true
             */
            std::string const & struct_member_name(uint32_t i) const {
                return this->td_->struct_member_name(i);
            }

        private:
            template<typename Fn>
            static void visit_graph_aux(TaggedPtr tp,
                                        Fn && visit_fn,
                                        std::unordered_set<void *> * p_visited_set)
                {
                    if (tp.address() == nullptr)
                        return;

                    if (p_visited_set->find(tp.address()) == p_visited_set->end()) {
                        p_visited_set->insert(tp.address());

                        visit_fn(tp);

                        for (uint32_t i = 0, n = tp.n_child(); i < n; ++i) {
                            visit_graph_aux(tp.get_child(i), visit_fn, p_visited_set);
                        }
                    }
                } /*visit_graph_aux*/

        private:
            friend class TaggedRcptr;

        private:
            /* describes the actual type stored at *address.
             * can be null if .address is null
             */
            TypeDescr td_;
            /* address with type information preserved at runtime */
            void * address_;
        }; /*TaggedPtr*/

    } /*namespace reflect*/
} /*namespace xo*/

/* end TaggedPtr.hpp */
