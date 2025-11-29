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

            // add2_i64
            {
                auto pm_expr = PrimitiveExpr_i64::make_add2_i64();

                gp<Object> rhs = xo::obj::make_primitive(mm, pm_expr->name(), pm_expr->value());

                rp<Variable> lhs = Variable::make(pm_expr->name(), pm_expr->value_td());
                gp<Object> * addr = env->establish_var(lhs.borrow());

                *addr = rhs;
            }
        }
    }
}

/* end BuiltinPrimitives.cpp */
