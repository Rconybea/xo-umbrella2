/** @file BuiltinPrimitives.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#include "xo/object/ObjectConverter.hpp"
#include "xo/alloc/IAlloc.hpp"
#include "GlobalEnv.hpp"

namespace xo {
    namespace scm {
        struct BuiltinPrimitives {
        public:
            using ObjectConverter = xo::obj::ObjectConverter;

            /** install conversions for PrimitiveExpr<Fn> -> Primitive<Fn>
             *  for particular function pointer types Fn.
             *
             *  Source type from xo-expression
             *  Dest type from xo-object.
             *
             *  Module dependence goes the other way
             *  i.e. xo-interpreter -uses-> xo-expression
             *                      -uses-> xo-object
             *  For this reason rejected adding a virtual method to PrimitiveExprInterface
             **/
            static void install_interpreter_conversions(ObjectConverter * target);

            static void install(gc::IAlloc * mm, gp<GlobalEnv> env);
        };
    }
}

/* end BuiltinPrimitives.hpp */
