/* file parser.cpp
 *
 * author: Roland Conybeare
 */

#include "parser.hpp"
#include "parserstatemachine.hpp"
#include "define_xs.hpp"
#include "exprseq_xs.hpp"
#include "pretty_expression.hpp"
#include "xo/expression/DefineExpr.hpp"
#include "xo/expression/Constant.hpp"
#include "xo/expression/ConvertExpr.hpp"
//#include "xo/expression/GlobalEnv.hpp"
#include "xo/expression/LocalEnv.hpp"
//#include <regex>
#include <stdexcept>

namespace xo {
    using xo::ast::Expression;
    //using xo::ast::GlobalEnv;
    using xo::ast::LocalEnv;
    //using xo::ast::DefineExpr;
    //using xo::ast::ConvertExpr;
    //using xo::ast::Constant;
    //using xo::reflect::Reflect;
    using xo::reflect::TypeDescr;

    namespace scm {
        // ----- parser -----

        parser::parser()
            : xs_stack_{}, env_stack_{}
        {
            /* top-level environment.  initially empty */
            rp<LocalEnv> toplevel_env = LocalEnv::make_empty();

            this->env_stack_.push_envframe(toplevel_env);
        }

        bool
        parser::has_incomplete_expr() const {
            /* (don't count toplevel exprseq) */
            return xs_stack_.size() > 1;
        }

        void
        parser::begin_interactive_session() {
            /* note: not using emit expr here */
            parserstatemachine psm(&xs_stack_,
                                   &env_stack_,
                                   nullptr /*p_emit_expr*/);

            exprseq_xs::start(exprseqtype::toplevel_interactive, &psm);
        }

        void
        parser::begin_translation_unit() {
            /* note: not using emit expr here */
            parserstatemachine psm(&xs_stack_,
                                   &env_stack_,
                                   nullptr /*p_emit_expr*/);

            exprseq_xs::start(exprseqtype::toplevel_batch, &psm);
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

            log && log(xtag("top", xs_stack_.top_exprstate()));

            rp<Expression> retval;

            parserstatemachine psm(&xs_stack_, &env_stack_, &retval);

            xs_stack_.top_exprstate().on_input(tk, &psm);

            log && log(xtag("retval", retval));

            return retval;
        } /*include_token*/

        void
        parser::reset_to_idle_toplevel()
        {
            xs_stack_.reset_to_toplevel();
            env_stack_.reset_to_toplevel();
        } /*discard_current_state*/

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
