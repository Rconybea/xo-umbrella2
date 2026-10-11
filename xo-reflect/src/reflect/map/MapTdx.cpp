/** @file MapTdx.cpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "map/MapTdx.hpp"
#include "TypeDescr.hpp"

namespace xo {
    namespace reflect {
        TaggedPtr
        MapTdx::child_tp(uint32_t i, void * object) const
        {
            TaggedPtr retval = TaggedPtr::universal_null();
            uint32_t k = 0;

            this->visit_entries(object,
                                [i, &k, &retval](TaggedPtr /*key*/, TaggedPtr value) {
                                    if (k == i)
                                        retval = value;
                                    ++k;
                                });

            return retval;
        } /*child_tp*/

        TypeDescr
        MapTdx::fixed_child_td(uint32_t /*i*/) const
        {
            return this->value_td();
        } /*fixed_child_td*/

        std::string const &
        MapTdx::struct_member_name(uint32_t i) const
        {
            return TypeDescrExtra::struct_member_name(i);
        } /*struct_member_name*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end MapTdx.cpp */
