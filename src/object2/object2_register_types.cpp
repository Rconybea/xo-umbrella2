/** @file object2_register_types.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "object2_register_types.hpp"

#include "IGCObject_DList.hpp"
#include "IGCObject_DFloat.hpp"
#include "IGCObject_DInteger.hpp"

#include "IPrintable_DList.hpp"
//#include "IPrintable_DFloat.hpp"
#include "IPrintable_DInteger.hpp"

#include <xo/facet/FacetRegistry.hpp>
#include <xo/indentlog/scope.hpp>

namespace xo {
//    using xo::print::APrintable;
//    using xo::mm::AAllocator;
    using xo::mm::ACollector;
    using xo::mm::AGCObject;
//    using xo::mm::IGCObject_Any;
//    using xo::facet::FacetRegistry;
    using xo::facet::impl_for;
    using xo::facet::typeseq;
    using xo::scope;

    namespace scm {
        bool
        object2_register_types(obj<ACollector> gc)
        {
            scope log(XO_DEBUG(true));

            bool ok = true;

            ok &= gc.install_type(impl_for<AGCObject, DList>());

            ok &= gc.install_type(impl_for<AGCObject, DFloat>());

            return ok;
        }
    }
} /*namespace xo*/

/* end object2_register_types.cpp */
