/** @file List.cpp
 *
 *  author: Roland Conybeare, Aug 2025
 **/

#include "List.hpp"
#include <cassert>
#include <cstddef>

namespace xo {
    namespace obj {
        List::List(gp<Object> head, gp<List> tail)
            : head_{head}, tail_{tail} {}

        gp<List>
        List::nil = new List(nullptr, nullptr);

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
                l = l->tail();
            }

            return retval;
        }

        gp<Object>
        List::list_ref(std::size_t i) const {
            gp<const List> rem(this);

            while (i > 0) {
                assert(!(rem->is_nil()));

                rem = rem->tail();
                --i;
            }

            return rem->head();

        }

        std::size_t
        List::_shallow_size() const {
            return sizeof(List);
        }

        Object *
        List::_shallow_copy() const {
            assert(!(this->is_nil()));

            Cpof cpof(this);

            return new (cpof) List(*this);
        }

        std::size_t
        List::_forward_children() {
            Object::_forward_inplace(head_);
            Object::_forward_inplace(tail_);
            return List::_shallow_size();
        }
    }
}

/* end List.cpp */
