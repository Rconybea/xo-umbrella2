/** @file object2_register_facets.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "object2_register_facets.hpp"

#include <xo/object2/array/IGCObject_DArray.hpp>
#include <xo/object2/list/IGCObject_DList.hpp>
#include <xo/object2/boolean/IGCObject_DBoolean.hpp>
#include <xo/object2/number/IGCObject_DFloat.hpp>
#include <xo/object2/number/IGCObject_DInteger.hpp>
#include <xo/object2/string/IGCObject_DString.hpp>

#include <xo/object2/array/IPrintable_DArray.hpp>
#include <xo/object2/list/IPrintable_DList.hpp>
#include <xo/object2/boolean/IPrintable_DBoolean.hpp>
#include <xo/object2/number/IPrintable_DFloat.hpp>
#include <xo/object2/number/IPrintable_DInteger.hpp>
#include <xo/object2/string/IPrintable_DString.hpp>

#include <xo/object2/list/ISequence_DList.hpp>
#include <xo/object2/array/ISequence_DArray.hpp>

#include <xo/printable2/detail/APrintable.hpp>
#include <xo/alloc2/alloc/AAllocator.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/indentlog/scope.hpp>

namespace xo {
    using xo::print::APrintable;
    using xo::mm::AAllocator;
    using xo::mm::AGCObject;
    using xo::scm::DList;
    using xo::scm::DBoolean;
    using xo::scm::DFloat;
    using xo::scm::DString;
    using xo::scm::DArray;
    using xo::facet::DVariantPlaceholder;
    using xo::facet::FacetRegistry;
    using xo::facet::typeseq;

    namespace scm {
        bool
        object2_register_facets()
        {
            scope log(XO_DEBUG(true));

            FacetRegistry::register_impl<AGCObject, DList>();
            FacetRegistry::register_impl<APrintable, DList>();
            FacetRegistry::register_impl<ASequence, DList>();

            FacetRegistry::register_impl<AGCObject, DBoolean>();
            FacetRegistry::register_impl<APrintable, DBoolean>();

            FacetRegistry::register_impl<AGCObject, DFloat>();
            FacetRegistry::register_impl<APrintable, DFloat>();

            FacetRegistry::register_impl<AGCObject, DInteger>();
            FacetRegistry::register_impl<APrintable, DInteger>();

            FacetRegistry::register_impl<AGCObject, DString>();
            FacetRegistry::register_impl<APrintable, DString>();

            FacetRegistry::register_impl<AGCObject, DArray>();
            FacetRegistry::register_impl<APrintable, DArray>();
            FacetRegistry::register_impl<ASequence, DArray>();

            log && log(xtag("DVariantPlaceholder.tseq", typeseq::id<DVariantPlaceholder>()));

            log && log(xtag("DList.tseq", typeseq::id<DList>()));
            log && log(xtag("DBoolean.tseq", typeseq::id<DBoolean>()));
            log && log(xtag("DFloat.tseq", typeseq::id<DFloat>()));
            log && log(xtag("DInteger.tseq", typeseq::id<DInteger>()));
            log && log(xtag("DString.tseq", typeseq::id<DString>()));
            log && log(xtag("DArray.tseq", typeseq::id<DArray>()));

            log && log(xtag("AAllocator.tseq", typeseq::id<AAllocator>()));
            log && log(xtag("APrintable.tseq", typeseq::id<APrintable>()));
            log && log(xtag("AGCObject.tseq", typeseq::id<AGCObject>()));
            log && log(xtag("ASequence.tseq", typeseq::id<ASequence>()));

            return true;
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end object2_register_facets.cpp */
