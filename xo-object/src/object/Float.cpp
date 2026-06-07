/** @file Float.cpp
 *
 *  @author: Roland Conybeare, Nov 2025
 **/

#include "Float.hpp"
#include "xo/reflect/Reflect.hpp"

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;

    namespace obj {
        static_assert(sizeof(Float::float_type) == 8, "expected 64-bit representation for xo::obj::Float");

        Float::Float(float_type x) : value_{x} {}

        gp<Float>
        Float::make(IAlloc * mm, float_type x)
        {
            return new (MMPtr(mm)) Float(x);
        }

        gp<Float>
        Float::from(gp<Object> x) {
            return dynamic_cast<Float*>(x.ptr());
        }

        TaggedPtr
        Float::self_tp() const {
            return Reflect::make_tp(const_cast<Float*>(this));
        }

        void
        Float::display(std::ostream & os) const {
            os << value_;
        }

        std::size_t
        Float::_shallow_size() const {
            return sizeof(Float);
        }

        Object *
        Float::_shallow_copy(gc::IAlloc * mm) const {
            Cpof cpof(mm, this);
            return new (cpof) Float(*this);
        }

        std::size_t
        Float::_forward_children(gc::IAlloc * /*gc*/) {
            return Float::_shallow_size();
        }
    }
}

/* end Float.cpp */
