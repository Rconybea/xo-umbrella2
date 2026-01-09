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
    using xo::print::APrintable;
    using xo::mm::AAllocator;
    using xo::mm::ACollector;
    using xo::mm::AGCObject;
    using xo::mm::IGCObject_Any;
    using xo::facet::FacetRegistry;
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

        bool
        object2_register_facets()
        {
            scope log(XO_DEBUG(true));

            FacetRegistry::register_impl<AGCObject, DList>();
            FacetRegistry::register_impl<APrintable, DList>();

            FacetRegistry::register_impl<AGCObject, DFloat>();
//            FacetRegistry::register_impl<APrintable, DFloat>();

            FacetRegistry::register_impl<AGCObject, DInteger>();
            FacetRegistry::register_impl<APrintable, DInteger>();

            log && log(xtag("DList.tseq", typeseq::id<DList>()));
            log && log(xtag("DFloat.tseq", typeseq::id<DFloat>()));
            log && log(xtag("DInteger.tseq", typeseq::id<DInteger>()));

            log && log(xtag("AAllocator.tseq", typeseq::id<AAllocator>()));
            log && log(xtag("APrintable.tseq", typeseq::id<APrintable>()));
            log && log(xtag("AGCObject.tseq", typeseq::id<AGCObject>()));

            return true;
        }
    }
} /*namespace xo*/

/* end object2_register_types.cpp */
