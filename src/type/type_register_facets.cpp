/** @file type_register_facets.cpp
 *
 * @author Roland Conybeare, Feb 2026
 **/

#include "type_register_facets.hpp"

#include <xo/type/AtomicType.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/indentlog/scope.hpp>

namespace xo {
    using xo::mm::AGCObject;
    using xo::facet::FacetRegistry;
    //using xo::facet::TypeRegistry;
    using xo::reflect::typeseq;

    namespace scm {

        bool
        type_register_facets()
        {
            scope log(XO_DEBUG(true));

            FacetRegistry::register_impl<AGCObject, DAtomicType>();

            log && log(xtag("DAtomicType.tseq", typeseq::id<DAtomicType>()));

            return true;
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end type_register_facets.cpp */
