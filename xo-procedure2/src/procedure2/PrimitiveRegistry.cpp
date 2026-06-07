/** @file PrimitiveRegistry.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "PrimitiveRegistry.hpp"
#include <xo/indentlog/scope.hpp>

namespace xo {
    namespace scm {
        PrimitiveRegistry &
        PrimitiveRegistry::instance()
        {
            static PrimitiveRegistry s_instance;

            return s_instance;
        }

        void
        PrimitiveRegistry::register_primitives(InstallSource factory)
        {
            scope log(XO_DEBUG(true));

            init_seq_v_.push_back(factory);
        }

        bool
        PrimitiveRegistry::install_primitives(obj<ARuntimeContext> rcx,
                                              //obj<AAllocator> mm,
                                              //StringTable * stbl,
                                              InstallSink sink,
                                              InstallFlags flags)
        {
            scope log(XO_DEBUG(false));

            bool ok = true;

            size_t i = 0;
            size_t n = init_seq_v_.size();
            log && log("run n init steps", xtag("n", n));

            for (const auto & fn : init_seq_v_) {
                log && log("do install fn (", i+1, "/", n, ")");

                ok = ok & fn(rcx, /*mm, stbl,*/ sink, flags);
                ++i;
            }

            return ok;
        }
    }
} /*namespace xo*/
