/* @file object.cpp */

#include "object.hpp"
#include "zstring.hpp"

namespace xo {
    namespace obj {
        object::object(const object & x) : value_{0} {
            switch (x.tag()) {
            case otag::ot_invalid:
            case otag::ot_sentinel:
            case otag::ot_boolean:
            case otag::ot_char:
            case otag::ot_i32:
            case otag::ot_f32:
            case otag::ot_i64:
            case otag::ot_f64:
            case otag::ot_zstring:
            case otag::ot_symbol:
            case otag::ot_cons:
                /* in most cases where value_ embeds a pointer,
                 * it's a passive garbage-collected pointer,
                 * no special treatment required here.
                 */

                value_ = x.value_;
                break;
            case otag::ot_rc_object:
                /* must bump refcount */
                {
                    Object * rc_obj = cast_object();

                    intrusive_ptr_add_ref(rc_obj);
                }
                break;
            }
        }
    } /*namespace obj*/
} /*namespace xo*/


/* end object.cpp */
