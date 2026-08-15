/** @file GCObjectConversion_DInteger.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "number/GCObjectConversion_DInteger.hpp"
#include <xo/ppsink/tag.hpp>
#include <xo/ppsink/tostr_xx.hpp>

namespace xo {
    /* the ppsink printing vocabulary, for use below */
    using xo::pp::tostr0;
    using xo::pp::xtag;

    using xo::mm::AGCObject;

    namespace scm {

        obj<AGCObject>
        GCObjectConversion<long>::to_gco(obj<AAllocator> mm,
                                           long x)
        {
            return DInteger::box<AGCObject>(mm, x);
        }

        long
        GCObjectConversion<long>::from_gco(obj<AAllocator> mm,
                                           obj<AGCObject> gco)
        {
            (void)mm;

            auto int_obj = obj<AGCObject,DInteger>::from(gco);

            if (!int_obj) {
                throw std::runtime_error
                    (tostr0("Object obj found where Integer expected",
                           xtag("obj", gco)));
            }

            return int_obj.data()->value();
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end GCObjectConversion_DFloat.cpp */
