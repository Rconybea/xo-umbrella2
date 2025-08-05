/* @file Integer.cpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "Integer.hpp"
#include <cstddef>

namespace xo {
    namespace obj {
        Integer::Integer(int_type x) : value_{x} {}

        gp<Integer>
        Integer::make(int_type x) {
            return new (MMPtr(mm)) Integer(x);
        }

        gp<Integer>
        Integer::from(gp<Object> x) {
            return dynamic_cast<Integer*>(x.ptr());
        }

        std::size_t
        Integer::_shallow_size() const {
            return sizeof(Integer);
        }

        Object *
        Integer::_shallow_copy() const {
            Cpof cpof(this);
            return new (cpof) Integer(*this);
        }

        std::size_t
        Integer::_forward_children() {
            return Integer::_shallow_size();
        }

    } /*namespace obj*/
} /*namespace xo*/

/* end Integer.cpp */
