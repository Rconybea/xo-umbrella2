/** @file BuiltinPrimitives.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#include "xo/alloc/IAlloc.hpp"
#include "GlobalEnv.hpp"

namespace xo {
    namespace scm {
        struct BuiltinPrimitives {
        public:
            static void install(gc::IAlloc * mm, gp<GlobalEnv> env);
        };
    }
}

/* end BuiltinPrimitives.hpp */
