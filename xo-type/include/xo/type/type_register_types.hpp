/** @file type_register_types.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <xo/alloc2/Collector.hpp>

namespace xo {
    namespace scm {
        /** Register type (facet,impl) combinations with FacetRegistry **/
        bool type_register_types(obj<xo::mm::ACollector> gc);
    }
}

/* end type_register_types.hpp */
