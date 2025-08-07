/* @file Boolean.cpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "Boolean.hpp"
#include "TaggedPtr.hpp"
#include "xo/reflect/Reflect.hpp"
#include <array>
#include <cassert>
#include <cstddef>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;

    namespace obj {
        gp<Boolean>
        Boolean::boolean_obj(bool x)
        {
            static std::array<gp<Boolean>, 2> s_boolean_v
                = {{ new Boolean{false}, new Boolean{true} }};

            return s_boolean_v[static_cast<std::size_t>(x)];
        }

        gp<Boolean>
        Boolean::true_obj()
        {
            return boolean_obj(true);
        }

        gp<Boolean>
        Boolean::false_obj()
        {
            return boolean_obj(false);
        }

        TaggedPtr
        Boolean::self_tp() const
        {
            return Reflect::make_tp(const_cast<Boolean*>(this));
        }

        void
        Boolean::display(std::ostream & os) const
        {
            os << (value_ ? "#t" : "#f");
        }

        std::size_t
        Boolean::_shallow_size() const
        {
            return sizeof(Boolean);
        }

        // LCOV_EXCL_START
        Object *
        Boolean::_shallow_copy() const
        {

            /* Boolean instances not created in GC-owned space,
             * so GC will not traverse them.
             *
             * If we wanted booleans in GC-owned space, would need
             * to pad Boolean::value_ with enough space to hold a forwarding
             * pointer
             */

            assert(false);
            return nullptr;

        }
        // LCOV_EXCL_STOP

        // LCOV_EXCL_START
        std::size_t
        Boolean::_forward_children()
        {
            assert(false);
            return 0;
        }
        // LCOV_EXCL_STOP
    }
} /*namespace xo*/

/* end Boolean.cpp */
