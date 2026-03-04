/** @file DPrimitive_gco_1_gco.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

#include <xo/alloc2/GCObject.hpp>
#include "DPrimitive.hpp"

namespace xo {
    namespace scm {
        using xo::mm::AGCObject;
        using xo::facet::obj;

        using DPrimitive_gco_1_gco = Primitive<obj<AGCObject> (*)(obj<ARuntimeContext>,
                                                                  obj<AGCObject>)>;
    }
}

/* end DPrimitive_gco_1_gco.hpp */
