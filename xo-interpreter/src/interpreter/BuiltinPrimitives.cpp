/** @file BuiltinPrimitives.cpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#include "BuiltinPrimitives.hpp"
#include "Integer.hpp"
#include "Primitive.hpp"
#include "xo/expression/PrimitiveExpr.hpp"
#include "xo/object/ObjectConversion.hpp"
#include "xo/reflect/Reflect.hpp"
#include <cstdint>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TypeDescr;

    namespace scm {
        int64_t
        add64(int64_t x, int64_t y)
        {
            return x + y;
        }

        void
        BuiltinPrimitives::install_interpreter_conversions(ObjectConverter * /*target*/)
        {
            /* abandoning this path */
        }

        void
        BuiltinPrimitives::install(gc::IAlloc * mm, gp<GlobalEnv> env)
        {
            scope log(XO_DEBUG(true));

            // add(x,y)
            {
                gp<Object> rhs = xo::obj::make_primitive(mm, "add", add64);

                TypeDescr td = Reflect::require_function<decltype(&add64)>();

                rp<Variable> lhs = Variable::make("add", td);
                gp<Object> * addr = env->establish_var(lhs.borrow());

                *addr = rhs;
            }

            // i64 comparisons

            // @cmp_eq2_i64
            install_pm(mm, PrimitiveExpr_cmp_i64::make_cmp_eq2_i64(), env);

            // @cmp_ne2_i64
            install_pm(mm, PrimitiveExpr_cmp_i64::make_cmp_ne2_i64(), env);

            // @cmp_lt2_i64
            install_pm(mm, PrimitiveExpr_cmp_i64::make_cmp_lt2_i64(), env);

            // @cmp_le2_i64
            install_pm(mm, PrimitiveExpr_cmp_i64::make_cmp_le2_i64(), env);

            // @cmp_gt2_i64
            install_pm(mm, PrimitiveExpr_cmp_i64::make_cmp_gt2_i64(), env);

            // @cmp_ge2_i64
            install_pm(mm, PrimitiveExpr_cmp_i64::make_cmp_ge2_i64(), env);

            // i64 arithmetic

            // @add2_i64
            install_pm(mm, PrimitiveExpr_i64::make_add2_i64(), env);

            // @sub2_i64
            install_pm(mm, PrimitiveExpr_i64::make_sub2_i64(), env);

            // @mul2_i64
            install_pm(mm, PrimitiveExpr_i64::make_mul2_i64(), env);

            // @div2_i64
            install_pm(mm, PrimitiveExpr_i64::make_div2_i64(), env);

            // ----------------------------------------------------------------

            // @add2_f64
            install_pm(mm, PrimitiveExpr_f64::make_add2_f64(), env);

            // @sub2_f64
            install_pm(mm, PrimitiveExpr_f64::make_sub2_f64(), env);

            // @mul2_f64
            install_pm(mm, PrimitiveExpr_f64::make_mul2_f64(), env);

            // @div2_f64
            install_pm(mm, PrimitiveExpr_f64::make_div2_f64(), env);
        }
    }
}

/* end BuiltinPrimitives.cpp */
