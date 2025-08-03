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

            bool is_nil() const { return this == nil.ptr(); }

            gp<Object> head() const { return head_; }
            gp<List> tail() const { return tail_; }

            std::size_t size() const;
            gp<Object> list_ref(std::size_t i) const;

            // inherited from Object..

            virtual std::size_t _shallow_size() const override;
            virtual Object * _shallow_copy() const override;
            virtual std::size_t _forward_children() override;

        private:
            List(gp<Object> head, gp<List> tail);

        private:
            gp<Object> head_;
            gp<List>   tail_;
        };
    }
} /*namespace xo*/
