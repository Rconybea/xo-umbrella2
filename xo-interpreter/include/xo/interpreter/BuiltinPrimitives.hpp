/** @file BuiltinPrimitives.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#include "xo/object/ObjectConverter.hpp"
#include "xo/alloc/IAlloc.hpp"
#include "Primitive.hpp"
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

            template <typename Expr>
            static void install_pm(gc::IAlloc * mm, rp<Expr> pm_expr, gp<GlobalEnv> env) {
                gp<Object> rhs
                    = xo::obj::make_primitive(mm, pm_expr->name(), pm_expr->value());

                /* store in env using this variable-expr */
                rp<Variable> lhs
                    = Variable::make(pm_expr->name(), pm_expr->value_td());

                gp<Object> * addr = env->establish_var(lhs.borrow());

                *addr = rhs;
            }

            static void install(gc::IAlloc * mm, gp<GlobalEnv> env);
        };
    }
}

/* end BuiltinPrimitives.hpp */
