/** @file List.cpp
 *
 *  author: Roland Conybeare, Aug 2025
 **/

#include "List.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
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
        List::from(gp<IObject> x) {
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
        List::self_tp() const
        {
            return Reflect::make_tp(const_cast<List*>(this));
        }

        void
        List::pretty(xo::pp::PpSink & sink) const
        {
            gp<List> l = const_cast<List*>(this);

            /* Framing follows Prettifier<std::array> in
             * xo-ppsink/include/xo/ppsink/pretty_array.hpp: put the opening
             * delimiter, begin() a group, split() between elements, then
             * end() before the closing delimiter.
             *
             * split(1) is what the old `os << " "` becomes: one space when the
             * list fits on a line -- so flat output is byte-identical -- and a
             * break point when it does not.  A long list could not break at
             * all before.
             *
             * Elements go through sink.pp(), i.e. Prettifier<gp<Object>>, so a
             * nested list nests structurally rather than flattening.
             */
            sink.put("(").begin();

            size_t i = 0;
            while (!l->is_nil()) {
                if (i > 0)
                    sink.split(1);

                sink.pp(l->head());

                l = l->rest();
                ++i;
            }

            sink.end().put(")");
        }

        std::size_t
        List::_shallow_size() const
        {
            return sizeof(List);
        }

        IObject *
        List::_shallow_copy(gc::IAlloc * gc) const
        {
            assert(!(this->is_nil()));

            Cpof cpof(gc, this);

            return new (cpof) List(*this);
        }

        std::size_t
        List::_forward_children(gc::IAlloc * gc)
        {
            Object::_forward_inplace(head_, gc);
            Object::_forward_inplace(rest_, gc);
            return List::_shallow_size();
        }
    }
}

/* end List.cpp */
