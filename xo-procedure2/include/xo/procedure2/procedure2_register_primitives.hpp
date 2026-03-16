/** @file procedure2_register_primitives.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

#include "PrimitiveRegistry.hpp"
#include <xo/alloc2/Collector.hpp>

namespace xo {
    namespace scm {
        /** Register primitive-factories **/
        bool procedure2_register_primitives(obj<ARuntimeContext> rcx,
                                            //obj<xo::mm::AAllocator> gc,
                                            //StringTable * stbl,
                                            InstallSink sink,
                                            InstallFlags flags);
    }
}

/* end procedure2_register_primitives.hpp */
