/** @file procedure2_register_types.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "procedure2_register_types.hpp"

#include "detail/IGCObject_DPrimitive_gco_2_gco_gco.hpp"
#include <xo/indentlog/scope.hpp>

namespace xo {
    using xo::mm::ACollector;
    using xo::mm::AGCObject;
    using xo::facet::impl_for;
    using xo::facet::typeseq;
    using xo::scope;

    namespace scm {
        bool
        procedure2_register_types(obj<ACollector> gc)
        {
            scope log(XO_DEBUG(true));

            bool ok = true;

            // (note: don't currently intend to support AGCObject for DSimpleRcx)

            ok &= gc.install_type(impl_for<AGCObject, DPrimitive_gco_2_gco_gco>());

            return ok;
        }
    }
} /*namespace xo*/

/* end procedure2_register_types.cpp */
