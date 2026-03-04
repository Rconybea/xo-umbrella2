/** @file stringtable2_register_types.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <xo/alloc2/Collector.hpp>

namespace xo {
    namespace scm {
        /** Register stringtable2 (facet,impl) combinations with FacetRegistry **/
        bool stringtable2_register_types(obj<xo::mm::ACollector> gc);
    }
}

/* end stringtable2_register_types.hpp */
