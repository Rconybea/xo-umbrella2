/* @file List.hpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "Sequence.hpp"

namespace xo {
    namespace obj {
        /** @class List
         *  @brief A list element -- aka cons cell
         **/
        class List : public Sequence {
        public:
            /** the empty list. unique sentinel object **/
            static gp<List> nil;

            /** @return list with first element @p car, and tail @p cdr **/
            static gp<List> cons(gp<Object> car, gp<List> cdr);

            /** @return list with single element @p x1 **/
            template <typename T>
            static gp<List> list(T && x1) {
                return List::cons(x1, nil);
            }

            /** @return list with elements @p x1, ..., @p rest in argument order **/
            template <typename T, typename... Rest>
            static gp<List> list(T && x1, Rest &&... rest) {
                return List::cons(x1, list(rest...));
            }

            /** @return true iff list is empty **/
            bool is_nil() const { return this == nil.ptr(); }

            gp<Object> head() const { return head_; }
            gp<List> rest() const { return rest_; }

            /** @return first element in list; synonym for @ref head **/
            gp<Object> car() const { return head_; }
            /** @return remainder of list after first element; synonym for @ref rest **/
            gp<Object> cdr() const { return rest_; }

            /** @return number of top-level elements in this list **/
            std::size_t size() const;
            gp<Object> list_ref(std::size_t i) const;

            void assign_head(gp<Object> head);
            void assign_rest(gp<List> rest);

            // inherited from Object..

            virtual std::size_t _shallow_size() const override;
            virtual Object * _shallow_copy() const override;
            virtual std::size_t _forward_children() override;

        private:
            List(gp<Object> head, gp<List> rest);

        private:
            gp<Object> head_;
            gp<List>   rest_;
        };
    }
} /*namespace xo*/
