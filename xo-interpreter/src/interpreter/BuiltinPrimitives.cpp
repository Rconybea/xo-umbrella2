/** @file BuiltinPrimitives.cpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#include "BuiltinPrimitives.hpp"
#include "ObjectConversion.hpp"
#include "Integer.hpp"
#include "Primitive.hpp"
#include "xo/reflect/Reflect.hpp"
#include <cstdint>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TypeDescr;

    namespace scm {
        int64_t
        add64(int64_t x, int64_t y)
        {
            return x + y;
        }

        void
        BuiltinPrimitives::install(gc::IAlloc * mm, gp<GlobalEnv> env)
        {
            {
                gp<Object> rhs = xo::obj::make_primitive(mm, add64);
                TypeDescr td = Reflect::require<decltype(add64)>();
                rp<Variable> lhs = Variable::make("add", td);
                gp<Object> * addr = env->establish_var(lhs.borrow());

                *addr = rhs;
            }
        }
    }
}

/* end BuiltinPrimitives.cpp */
