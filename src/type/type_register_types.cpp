/** @file type_register_types.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "type_register_types.hpp"

#include "AtomicType.hpp"
#include <xo/alloc2/Collector.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/indentlog/scope.hpp>

namespace xo {
    using xo::mm::ACollector;
    using xo::mm::AGCObject;
    using xo::facet::impl_for;
    using xo::scope;

    namespace scm {
        bool
        type_register_types(obj<ACollector> gc)
        {
            scope log(XO_DEBUG(true));

            bool ok = true;

            ok &= gc.install_type(impl_for<AGCObject, DAtomicType>());

            return ok;
        }
    }
} /*namespace xo*/

/* end type_register_types.cpp */
