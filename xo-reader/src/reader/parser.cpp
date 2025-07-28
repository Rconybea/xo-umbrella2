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
    using xo::ast::LocalEnv;
    using xo::reflect::TypeDescr;

    namespace scm {
        // ----- parser -----

        parser::parser(bool debug_flag)
            : psm_{debug_flag}
        {
            /* top-level environment.  initially empty */
            rp<LocalEnv> toplevel_env = LocalEnv::make_empty();

            this->psm_.env_stack_.push_envframe(toplevel_env);
        }

        bool
        parser::has_incomplete_expr() const {
            /* (don't count toplevel exprseq) */
            return psm_.xs_stack_.size() > 1;
        }

        void
        parser::begin_interactive_session() {
            exprseq_xs::start(exprseqtype::toplevel_interactive, &psm_);
        }

        void
        parser::begin_translation_unit() {
            exprseq_xs::start(exprseqtype::toplevel_batch, &psm_);
        }

        const parser_result &
        parser::include_token(const token_type & tk)
        {
            scope log(XO_DEBUG(psm_.debug_flag()), xtag("tk", tk));

            if (psm_.xs_stack_.empty()) {
                throw std::runtime_error(tostr("parser::include_token",
                                                ": parser not expecting input"
                                               "(call parser.begin_translation_unit()..?)",
                                               xtag("token", tk)));
            }

            /* stack_ is non-empty */

            log && log(xrefrtag("top", psm_.xs_stack_.top_exprstate()));

            psm_.xs_stack_.top_exprstate().on_input(tk, &psm_);

            return psm_.result_;
        } /*include_token*/

        void
        parser::reset_to_idle_toplevel()
        {
            psm_.xs_stack_.reset_to_toplevel();
            psm_.env_stack_.reset_to_toplevel();
            psm_.result_ = parser_result::none();
        } /*discard_current_state*/

        void
        parser::print(std::ostream & os) const {
            os << "<parser"
               << std::endl;

            psm_.xs_stack_.print(os);

            os << ">" << std::endl;
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end parser.cpp */
