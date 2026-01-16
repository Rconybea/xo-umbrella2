/** @file CollectorTypeRegistry.cpp
 **/

#include "CollectorTypeRegistry.hpp"
#include <xo/indentlog/scope.hpp>

namespace xo {
    namespace mm {
        CollectorTypeRegistry &
        CollectorTypeRegistry::instance() {
            static CollectorTypeRegistry s_instance;

            return s_instance;
        }

        void
        CollectorTypeRegistry::register_types(init_function_type fn) {
            scope log(XO_DEBUG(true));

            init_seq_v_.push_back(fn);
        }

        bool
        CollectorTypeRegistry::install_types(obj<ACollector> gc) {
            scope log(XO_DEBUG(true));

            bool ok = true;

            size_t i = 0;
            size_t n = init_seq_v_.size();
            log && log("run n init steps", xtag("n", n));

            for (const auto & fn : init_seq_v_) {
                log && log("do install fn (", i+1, "/", n, ")");

                ok = ok & fn(gc);
            }

            return ok;
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end CollectorTypeRegistry.cpp */
