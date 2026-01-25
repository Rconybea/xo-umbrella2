/** @file GCObjectConversion_DFloat.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "number/GCObjectConversion_DFloat.hpp"
#include <xo/indentlog/print/tag.hpp>

namespace xo {
    using xo::mm::AGCObject;

    namespace scm {

        obj<AGCObject>
        GCObjectConversion<double>::to_gco(obj<AAllocator> mm,
                                           const double & x)
        {
            return DFloat::box<AGCObject>(mm, x);
        }

        double
        GCObjectConversion<double>::from_gco(obj<AAllocator> mm,
                                             obj<AGCObject> gco)
        {
            (void)mm;

            auto float_obj = obj<AGCObject,DFloat>::from(gco);

            if (!float_obj) {
                throw std::runtime_error
                    (tostr("Object obj found where Float expected",
                           xtag("obj", gco)));
            }

            return float_obj.data()->value();
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end GCObjectConversion_DFloat.cpp */
