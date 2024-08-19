/* @file paren_xs.cpp */

#include "paren_xs.hpp"
#include "parserstatemachine.hpp"
#include "exprstatestack.hpp"
#include "progress_xs.hpp"
#include "expect_expr_xs.hpp"

namespace xo {
    namespace scm {
        paren_xs::paren_xs()
            : parenxs_type_{parenexprstatetype::lparen_0}
        {}

        std::unique_ptr<paren_xs>
        paren_xs::make() {
            return std::make_unique<paren_xs>(paren_xs());
        }

        void
        paren_xs::start(exprstatestack * p_stack)
        {
            p_stack->push_exprstate(paren_xs::make());
            expect_expr_xs::start(p_stack);
        }

        bool
        paren_xs::admits_rightparen() const {
            switch (parenxs_type_) {
            case parenexprstatetype::lparen_0:
                /* unreachable */
                assert(false);
                return false;

            case parenexprstatetype::lparen_1:
                return true;

            case parenexprstatetype::invalid:
            case parenexprstatetype::n_parenexprstatetype:
                /* unreachable */
                assert(false);
                return false;
            }

            return false;
        }

        bool
        paren_xs::admits_f64() const {
            switch (parenxs_type_) {
            case parenexprstatetype::lparen_0:
                return true;

            case parenexprstatetype::lparen_1:
                return false;

            case parenexprstatetype::invalid:
            case parenexprstatetype::n_parenexprstatetype:
                /* unreachable */
                assert(false);
                return false;
            }

            return false;
        }

        void
        paren_xs::on_def_token(const token_type & tk,
                               parserstatemachine * /*p_psm*/)
        {
            constexpr const char * c_self_name = "paren_xs::on_def";

            this->illegal_input_error(c_self_name, tk);
        }

        void
        paren_xs::on_symbol_token(const token_type & /*tk*/,
                                  parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            auto p_stack = p_psm->p_stack_;

            log && log(xtag("exstype", p_stack->top_exprstate().exs_type()));

            //constexpr const char * self_name = "paren_xs::on_symbol";

            /* TODO: lparen_0: treat as variable reference */

            assert(false);
        }

        void
        paren_xs::on_typedescr(TypeDescr /*td*/,
                               parserstatemachine * /*p_psm*/)
        {
            assert(false);
            return;
        }

        void
        paren_xs::on_colon_token(const token_type & tk,
                                 parserstatemachine * /*p_psm*/)
        {
            constexpr const char * c_self_name = "paren_xs::on_colon";

            this->illegal_input_error(c_self_name, tk);
        }

        void
        paren_xs::on_semicolon_token(const token_type & tk,
                                     parserstatemachine * /*p_psm*/)
        {
            constexpr const char * c_self_name = "paren_xs::on_semicolon";

            this->illegal_input_error(c_self_name, tk);
        }

        void
        paren_xs::on_singleassign_token(const token_type & tk,
                                        parserstatemachine * /*p_psm*/)
        {
            constexpr const char * c_self_name = "paren_xs::on_singleassign";

            this->illegal_input_error(c_self_name, tk);
        }

        void
        paren_xs::on_leftparen_token(const token_type & tk,
                                     parserstatemachine * /*p_psm*/)
        {
            constexpr const char * c_self_name = "paren_xs::on_leftparen";

            this->illegal_input_error(c_self_name, tk);
        }

        void
        paren_xs::on_rightparen_token(const token_type & tk,
                                      parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * c_self_name = "paren_xs::on_rightparen";

            if (!this->admits_rightparen())
            {
                this->illegal_input_error(c_self_name, tk);
            }

            if (this->parenxs_type_ == parenexprstatetype::lparen_1) {
                rp<Expression> expr = this->gen_expr_;

                auto p_stack = p_psm->p_stack_;

                std::unique_ptr<exprstate> self = p_psm->pop_exprstate();

                p_stack->top_exprstate().on_expr(expr, p_psm);
            }
        }

        void
        paren_xs::on_f64_token(const token_type & tk,
                               parserstatemachine * /*p_psm*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * c_self_name = "paren_xs::on_f64";

            this->illegal_input_error(c_self_name, tk);
        }

        void
        paren_xs::on_expr(ref::brw<Expression> expr,
                          parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("exstype", this->exs_type_),
                       xtag("expr", expr));

            switch (this->parenxs_type_) {
            case parenexprstatetype::lparen_0: {
                this->parenxs_type_ = parenexprstatetype::lparen_1; /* wants on_rightparen */
                auto p_stack = p_psm->p_stack_;

                progress_xs::start(expr.promote(), p_stack);

                return;
            }

            case parenexprstatetype::lparen_1: {
                this->gen_expr_ = expr.promote();

                /* expect immediate incoming call, this time to on_rightparen() */
                return;
            }

            default:
                /* unreachable */
                assert(false);
                return;
            }
        } /*on_expr*/

        void
        paren_xs::on_symbol(const std::string & /*symbol_name*/,
                            parserstatemachine * /*p_psm*/)
        {
            switch(this->parenxs_type_) {
            case parenexprstatetype::lparen_0:
            case parenexprstatetype::lparen_1:
                /* NOT IMPLEMENTED */
                assert(false);
                return;

            default:
                /* unreachable */
                assert(false);
                return;
            }
        }

        void
        paren_xs::print(std::ostream & os) const {
            os << "<paren_xs"
               << xtag("type", exs_type_);
            if (gen_expr_)
                os << xtag("gen_expr", gen_expr_);
            os << ">";
        }


    } /*namespace scm*/
} /*namespace xo*/


/* end paren_xs.cpp */
