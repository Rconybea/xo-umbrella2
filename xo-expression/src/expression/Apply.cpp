/* @file Apply.cpp */

#include "Apply.hpp"
#include "Primitive.hpp"
#include "xo/indentlog/print/vector.hpp"

namespace xo {
    namespace ast {
        rp<Apply>
        Apply::make(const rp<Expression> & fn,
                    const std::vector<rp<Expression>> & argv)
        {
            /* extract result type from function type */
            TypeDescr fn_valuetype = fn->valuetype();

            if (!fn_valuetype->is_function()) {
                throw std::runtime_error
                    (tostr("Apply::make: found expression F in function position,"
                           " with value-type FT where a function type expected",
                           xtag("FT", fn_valuetype->short_name()),
                           xtag("F", fn_valuetype)));
            }

            TypeDescr fn_retval_type = fn_valuetype->fn_retval();

            return new Apply(fn_retval_type, fn, argv);
        }

        rp<Apply>
        Apply::make_add2_f64(const rp<Expression> & lhs,
                             const rp<Expression> & rhs)
        {
            return Apply::make(Primitive_f64::make_add2_f64(),
                               {lhs, rhs});
        }

        rp<Apply>
        Apply::make_sub2_f64(const rp<Expression> & lhs,
                             const rp<Expression> & rhs)
        {
            return Apply::make(Primitive_f64::make_sub2_f64(),
                               {lhs, rhs});
        }

        rp<Apply>
        Apply::make_mul2_f64(const rp<Expression> & lhs,
                             const rp<Expression> & rhs)
        {
            return Apply::make(Primitive_f64::make_mul2_f64(),
                               {lhs, rhs});
        }

        rp<Apply>
        Apply::make_div2_f64(const rp<Expression> & lhs,
                             const rp<Expression> & rhs)
        {
            return Apply::make(Primitive_f64::make_div2_f64(),
                               {lhs, rhs});
        }

        void
        Apply::display(std::ostream & os) const {
            os << "<Apply"
               << xtag("fn", fn_)
               << xtag("argv", argv_)
               << ">";
        }
    } /*namespace ast*/
} /*namespace xo*/


/* end Apply.cpp */
