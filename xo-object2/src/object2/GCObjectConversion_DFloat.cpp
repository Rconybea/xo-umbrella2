/** @file GCObjectConversion_DFloat.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "number/GCObjectConversion_DFloat.hpp"
#include <xo/ppsink/tag.hpp>
#include <xo/ppsink/tostr.hpp>

namespace xo {
    /* the ppsink printing vocabulary, for use below */
    using xo::pp::tostr;
    using xo::pp::xtag;

    using xo::mm::AGCObject;

    namespace scm {

        obj<AGCObject>
        GCObjectConversion<double>::to_gco(obj<AAllocator> mm,
                                           double x)
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
