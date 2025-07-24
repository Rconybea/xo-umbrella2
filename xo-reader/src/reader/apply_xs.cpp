/* @file apply_xs.cpp */

#include "apply_xs.hpp"
#include "parserstatemachine.hpp"
#include "expect_expr_xs.hpp"

namespace xo {
    namespace scm {
        // ----- applyexprstatetype -----

        const char *
        applyexprstatetype_descr(applyexprstatetype x) {
            switch (x) {
            case applyexprstatetype::invalid: return "invalid";
            case applyexprstatetype::apply_0: return "apply_0";
            case applyexprstatetype::apply_1: return "apply_1";
            case applyexprstatetype::apply_2: return "apply_2";
            case applyexprstatetype::apply_3: return "apply_3";
            case applyexprstatetype::n_applyexprstatetype: break;
            }

            return "???applyexprstatetype";
        }

        std::ostream &
        operator<<(std::ostream & os, applyexprstatetype x) {
            os << applyexprstatetype_descr(x);
            return os;
        }

        // ----- apply_xs -----

        std::unique_ptr<apply_xs>
        apply_xs::make() {
            return std::make_unique<apply_xs>(apply_xs());
        }

        void
        apply_xs::start(rp<Expression> fn_expr,
                        parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            p_psm->push_exprstate(apply_xs::make());
            p_psm->top_exprstate().on_expr(fn_expr.get(), p_psm);
            p_psm->top_exprstate().on_leftparen_token(token_type::leftparen(), p_psm);
        }

        apply_xs::apply_xs()
            : exprstate(exprstatetype::applyexpr)
        {}

        const char *
        apply_xs::get_expect_str() const {
            switch(applyxs_type_) {
            case applyexprstatetype::invalid: return "invalid";
            case applyexprstatetype::apply_0: return "expr";
            case applyexprstatetype::apply_1: return "lparen";
            case applyexprstatetype::apply_2: return "expr";
            case applyexprstatetype::apply_3: return "comma|rparen";
            case applyexprstatetype::n_applyexprstatetype: break;
            }

            return "?expect";
        }

        void
        apply_xs::on_expr(bp<Expression> expr,
                          parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            switch (applyxs_type_) {
            case applyexprstatetype::invalid:
            case applyexprstatetype::n_applyexprstatetype:
                // unreachable
                break;
            case applyexprstatetype::apply_0:
                this->fn_expr_ = expr.promote();
                this->applyxs_type_ = applyexprstatetype::apply_1;
                return;
            case applyexprstatetype::apply_1:
                // error, expecting lparen
                break;
            case applyexprstatetype::apply_2:
                this->args_expr_v_.push_back(expr.promote());
                this->applyxs_type_ = applyexprstatetype::apply_3;
                return;
            case applyexprstatetype::apply_3:
                // error, expecting comma|rparen
                break;
            }

            /* control here --implies-> error state */

            constexpr const char * c_self_name = "apply_xs::on_expr";
            const char * exp = this->get_expect_str();

            this->illegal_input_on_expr(c_self_name, expr, exp, p_psm);
        }

        void
        apply_xs::on_comma_token(const token_type & tk,
                                 parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            if (this->applyxs_type_ == applyexprstatetype::apply_3) {
                this->applyxs_type_ = applyexprstatetype::apply_2;
            } else {
                constexpr const char * c_self_name = "apply_xs::on_comma_token";
                const char * exp = this->get_expect_str();

                this->illegal_input_on_token(c_self_name, tk, exp, p_psm);
            }
        }

        void
        apply_xs::on_leftparen_token(const token_type & tk,
                                     parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            log && log("applyxs_type", applyxs_type_);

            if (this->applyxs_type_ == applyexprstatetype::apply_1) {
                this->applyxs_type_ = applyexprstatetype::apply_2;
            } else {
                constexpr const char * c_self_name = "apply_xs::on_leftparen_token";
                const char * exp = this->get_expect_str();

                this->illegal_input_on_token(c_self_name, tk, exp, p_psm);
            }
        }

        void
        apply_xs::on_rightparen_token(const token_type & tk,
                                      parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            log && log("applyxs_type", applyxs_type_);

            if (this->applyxs_type_ == applyexprstatetype::apply_3) {
                /* (done) state */

                rp<Apply> apply_expr = Apply::make(this->fn_expr_, this->args_expr_v_);

                std::unique_ptr<exprstate> self = p_psm->pop_exprstate();

                p_psm->top_exprstate().on_expr(apply_expr, p_psm);
                return;
            }

            constexpr const char * c_self_name = "apply_xs::on_rightparen_token";
            const char * exp = this->get_expect_str();

            this->illegal_input_on_token(c_self_name, tk, exp, p_psm);
        }

        void
        apply_xs::print(std::ostream & os) const
        {
            os << "<apply_xs"
               << xtag("this", (void*)this)
               << xtag("applyxs_type", applyxs_type_);
            os << ">";
        }

    } /*namespace scm*/
} /*namespace xo*/


/* end apply_xs.cpp */
