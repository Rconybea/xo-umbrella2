/* @file Apply.cpp */

#include "Apply.hpp"
#include "Primitive.hpp"
#include "exprtype.hpp"
#include "pretty_expression.hpp"
#include "xo/indentlog/print/vector.hpp"
#include "xo/indentlog/print/pretty_vector.hpp"
#include <cstdint>

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
        Apply::attach_envs(bp<Environment> p) {
            fn_->attach_envs(p);

            for (const auto & arg : argv_)
                arg->attach_envs(p);
        }

        void
        Apply::display(std::ostream & os) const {
            os << "<Apply"
               << xtag("fn", fn_)
               << xtag("argv", argv_)
               << ">";
        }

        std::uint32_t
        Apply::pretty_print(const ppindentinfo & ppii) const
        {
            return ppii.pps()->pretty_struct(ppii, "Apply",
                                             refrtag("fn", fn_),
                                             refrtag("argv", argv_));

#ifdef OBSOLETE
            ppstate * pps = ppii.pps();

            if (ppii.upto()) {
                if (!pps->print_upto("<Apply"))
                    return false;

                if (!pps->print_upto_tag("fn", fn_))
                    return false;

                if (!pps->print_upto_tag("argv", argv_))
                    return false;

                return true;
            } else {
                pps->write("<Apply");
                pps->newline_pretty_tag(ppii.ci1(), "fn", fn_);
                pps->newline_pretty_tag(ppii.ci1(), "argv", argv_);
                pps->write(">");

                return false;
            }
#endif
        }

    } /*namespace ast*/
} /*namespace xo*/


/* end Apply.cpp */
