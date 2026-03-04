/** @file DPrimitive_gco_0.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include <xo/alloc2/GCObject.hpp>
#include "DPrimitive.hpp"

namespace xo {
    namespace scm {
        using xo::mm::AGCObject;
        using xo::facet::obj;

        using DPrimitive_gco_0 = Primitive<obj<AGCObject> (*)(obj<ARuntimeContext>)>;
    }
}

/* end DPrimitive_gco_0.hpp */
