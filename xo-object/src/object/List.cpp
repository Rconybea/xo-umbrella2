/** @file List.cpp
 *
 *  author: Roland Conybeare, Aug 2025
 **/

#include "List.hpp"
#include "xo/reflect/Reflect.hpp"
#include "xo/indentlog/scope.hpp"
#include <cassert>
#include <cstddef>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;

    namespace obj {
        List::List(gp<Object> head, gp<List> rest)
            : head_{head}, rest_{rest} {}

        gp<List>
        List::nil = new List(nullptr, nullptr);

        gp<List>
        List::from(gp<Object> x) {
            return dynamic_cast<List *>(x.ptr());
        }

        gp<List>
        List::cons(gp<Object> car, gp<List> cdr) {
            return new (MMPtr(mm)) List(car, cdr);
        }

        std::size_t
        List::size() const {
            std::size_t retval = 0;

            gp<const List> l(this);
            while (!l->is_nil()) {
                ++retval;
                l = l->rest();
            }

            return retval;
        }

        gp<Object>
        List::list_ref(std::size_t i) const {
            gp<const List> rem(this);

            while (i > 0) {
                assert(!(rem->is_nil()));

                rem = rem->rest();
                --i;
            }

            return rem->head();

        }

        void
        List::assign_head(gp<Object> head)
        {
            Object::assign_member(this, &(this->head_), head);
        }

        void
        List::assign_rest(gp<List> tail)
        {
            Object::assign_member(this, &(this->rest_), tail);
        }

        TaggedPtr
        List::self_tp() const {
            return Reflect::make_tp(const_cast<List*>(this));
        }

        std::size_t
        List::_shallow_size() const {
            return sizeof(List);
        }

        Object *
        List::_shallow_copy() const {
            scope log(XO_DEBUG(Object::mm->debug_flag()));

            assert(!(this->is_nil()));

            Cpof cpof(this);

            return new (cpof) List(*this);
        }

        std::size_t
        List::_forward_children() {
            Object::_forward_inplace(head_);
            Object::_forward_inplace(rest_);
            return List::_shallow_size();
        }
    }
}

/* end List.cpp */
