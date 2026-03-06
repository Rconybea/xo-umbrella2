/** @file type_register_facets.cpp
 *
 * @author Roland Conybeare, Feb 2026
 **/

#include "type_register_facets.hpp"

#include <xo/type/AtomicType.hpp>
#include <xo/type/ListType.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/indentlog/scope.hpp>

namespace xo {
    using xo::mm::AGCObject;
    using xo::facet::FacetRegistry;
    using xo::reflect::typeseq;

    namespace scm {

        bool
        type_register_facets()
        {
            scope log(XO_DEBUG(true));

            FacetRegistry::register_impl<AType, DAtomicType>();
            FacetRegistry::register_impl<AGCObject, DAtomicType>();

            FacetRegistry::register_impl<AType, DListType>();
            FacetRegistry::register_impl<AGCObject, DListType>();

            log && log(xtag("DAtomicType.tseq", typeseq::id<DAtomicType>()));
            log && log(xtag("DListType.tseq", typeseq::id<DListType>()));

            return true;
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end type_register_facets.cpp */
