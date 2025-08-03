/* @file Boolean.cpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "Boolean.hpp"
#include <array>
#include <cassert>
#include <cstddef>

namespace xo {
    namespace obj {
        gp<Boolean>
        Boolean::boolean_obj(bool x)
        {
            static std::array<gp<Boolean>, 2> s_boolean_v
                = {{ new Boolean{false}, new Boolean{true} }};

            return s_boolean_v[static_cast<std::size_t>(x)];
        }

        std::size_t
        Boolean::_shallow_size() const
        {
            return sizeof(Boolean);
        }

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

        std::size_t
        Boolean::_forward_children()
        {
            assert(false);
            return 0;
        }
    }
} /*namespace xo*/

/* end Boolean.cpp */
