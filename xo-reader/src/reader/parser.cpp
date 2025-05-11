/* file parser.cpp
 *
 * author: Roland Conybeare
 */

#include "parser.hpp"
#include "parserstatemachine.hpp"
#include "define_xs.hpp"
#include "exprseq_xs.hpp"
#include "xo/expression/DefineExpr.hpp"
#include "xo/expression/Constant.hpp"
#include "xo/expression/ConvertExpr.hpp"
//#include <regex>
#include <stdexcept>

namespace xo {
    using xo::ast::Expression;
    //using xo::ast::DefineExpr;
    //using xo::ast::ConvertExpr;
    //using xo::ast::Constant;
    //using xo::reflect::Reflect;
    using xo::reflect::TypeDescr;

    namespace scm {
        // ----- parser -----

        bool
        parser::has_incomplete_expr() const {
            return !xs_stack_.empty();
        }

        void
        parser::begin_translation_unit() {
            /* note: not using emit expr here */
            parserstatemachine psm(&xs_stack_,
                                   &env_stack_,
                                   nullptr /*p_emit_expr*/);

            exprseq_xs::start(&psm);
        }

        rp<Expression>
        parser::include_token(const token_type & tk)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag), xtag("tk", tk));

            if (xs_stack_.empty()) {
                throw std::runtime_error(tostr("parser::include_token",
                                                ": parser not expecting input"
                                               "(call parser.begin_translation_unit()..?)",
                                               xtag("token", tk)));
            }

            /* stack_ is non-empty */

            rp<Expression> retval;

            parserstatemachine psm(&xs_stack_, &env_stack_, &retval);

            xs_stack_.top_exprstate().on_input(tk, &psm);

            log && log(xtag("retval", retval));

            return retval;
        } /*include_token*/

        void
        parser::print(std::ostream & os) const {
            os << "<parser"
               << std::endl;

            xs_stack_.print(os);

            os << ">" << std::endl;
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end parser.cpp */
