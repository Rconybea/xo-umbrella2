/** @file object2_register_facets.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "object2_register_facets.hpp"
#include <xo/object2/IGCObject_DList.hpp>
#include <xo/object2/IGCObject_DFloat.hpp>
#include <xo/object2/IGCObject_DInteger.hpp>

#include <xo/object2/IPrintable_DList.hpp>
#include <xo/object2/IPrintable_DFloat.hpp>
#include <xo/object2/IPrintable_DInteger.hpp>

#include <xo/printable2/detail/APrintable.hpp>
#include <xo/alloc2/alloc/AAllocator.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/indentlog/scope.hpp>

namespace xo {
    using xo::print::APrintable;
    using xo::mm::AAllocator;
    using xo::mm::AGCObject;
    using xo::scm::DList;
    using xo::scm::DFloat;
    using xo::facet::FacetRegistry;
    using xo::facet::typeseq;

    namespace scm {
        bool
        object2_register_facets()
        {
            scope log(XO_DEBUG(true));

            FacetRegistry::register_impl<AGCObject, DList>();
            FacetRegistry::register_impl<APrintable, DList>();

            FacetRegistry::register_impl<AGCObject, DFloat>();
            FacetRegistry::register_impl<APrintable, DFloat>();

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
    } /*namespace scm*/
} /*namespace xo*/

/* end object2_register_facets.cpp */
