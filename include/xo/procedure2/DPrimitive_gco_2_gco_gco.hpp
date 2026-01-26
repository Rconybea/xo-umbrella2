/** @file DPrimitive_gco_2_gco_gco.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include <xo/gc/GCObject.hpp>
#include "DPrimitive.hpp"

namespace xo {
    namespace scm {
        using xo::mm::AGCObject;
        using xo::facet::obj;

        using DPrimitive_gco_2_gco_gco = Primitive<obj<AGCObject> (*)(obj<ARuntimeContext>,
                                                                      obj<AGCObject>,
                                                                      obj<AGCObject>)>;
    }
}

/* end DPrimitive_gco_2_gco_gco.hpp */
