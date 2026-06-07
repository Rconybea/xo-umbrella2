/* @file Integer.cpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "Integer.hpp"
#include "xo/reflect/Reflect.hpp"
#include <cstddef>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;
    using xo::gc::IAlloc;

    namespace obj {
        static_assert(sizeof(Integer::int_type) == 8, "expected 64-bit representation for xo::obj::Integer");

        Integer::Integer(int_type x) : value_{x} {}

        gp<Integer>
        Integer::make(IAlloc * mm, int_type x) {
            return new (MMPtr(mm)) Integer(x);
        }

        gp<Integer>
        Integer::from(gp<IObject> x) {
            return gp<Integer>::from(x);
        }

        TaggedPtr
        Integer::self_tp() const {
            return Reflect::make_tp(const_cast<Integer*>(this));
        }

        void
        Integer::display(std::ostream & os) const {
            os << value_;
        }

        std::size_t
        Integer::_shallow_size() const {
            return sizeof(Integer);
        }

        Object *
        Integer::_shallow_copy(gc::IAlloc * mm) const {
            Cpof cpof(mm, this);
            return new (cpof) Integer(*this);
        }

        std::size_t
        Integer::_forward_children(gc::IAlloc * /*gc*/) {
            return Integer::_shallow_size();
        }

    } /*namespace obj*/
} /*namespace xo*/

/* end Integer.cpp */
