/** @file procedure2_register_types.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <xo/gc/Collector.hpp>

namespace xo {
    namespace scm {
        /** Register gc-aware (AGCObject,DRepr) combinations with garbage collector @p gc **/
        bool procedure2_register_types(obj<xo::mm::ACollector> gc);
    }
}

/* end procedure2_register_types.hpp */
