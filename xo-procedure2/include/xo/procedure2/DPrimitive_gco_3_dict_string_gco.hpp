/** @file DPrimitive_gco_3_dict_string_gco.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

#include <xo/object2/Dictionary.hpp>
#include <xo/object2/String.hpp>
#include <xo/gc/GCObject.hpp>
#include "DPrimitive.hpp"

namespace xo {
    namespace scm {
        using xo::mm::AGCObject;
        using xo::facet::obj;

        using DPrimitive_gco_3_dict_string_gco
        = Primitive<obj<AGCObject> (*)(obj<ARuntimeContext>,
                                       obj<AGCObject,DDictionary>,
                                       obj<AGCObject,DString>,
                                       obj<AGCObject>)>;
    }
}

/* end DPrimitive_gco_3_dict_string_gco.hpp */
