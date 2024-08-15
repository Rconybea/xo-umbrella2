/* @file progress_xs.cpp */

#include "progress_xs.hpp"
#include "expect_expr_xs.hpp"
#include "xo/expression/Apply.hpp"

namespace xo {
    using xo::ast::Apply;

    namespace scm {
        std::unique_ptr<progress_xs>
        progress_xs::make(rp<Expression> valex) {
            return std::make_unique<progress_xs>(progress_xs(std::move(valex)));
        }

        progress_xs::progress_xs(rp<Expression> valex)
            : exprstate(exprstatetype::expr_progress),
              gen_expr_{std::move(valex)}
        {}

        bool
        progress_xs::admits_f64() const { return false; }

        void
        progress_xs::on_def_token(const token_type & tk,
                                  exprstatestack * /*p_stack*/)
        {
            constexpr const char * self_name = "progress_xs::on_def";

            /* nothing here - admits_definition unconditionally false */
            this->illegal_input_error(self_name, tk) ;
        }

        void
        progress_xs::on_expr(ref::brw<Expression> expr,
                             exprstatestack * p_stack,
                             rp<Expression> * /*p_emit_expr*/)
        {
            constexpr const char * c_self_name = "progress_xs::on_expr";

            rp<Expression> result;

            /* consecutive expressions isn't legal */
            switch (op_type_) {
            case optype::invalid:
                throw std::runtime_error(tostr(c_self_name,
                                               ": consecutive unseparated exprs not legal"));

                break;

            case optype::op_add:
                result = Apply::make_add2_f64(this->gen_expr_ /*lhs*/,
                                              expr.promote() /*rhs*/);
                break;

            case optype::op_subtract:
                result = Apply::make_sub2_f64(this->gen_expr_ /*lhs*/,
                                              expr.promote() /*rhs*/);
                break;

            case optype::op_multiply:
                result = Apply::make_mul2_f64(this->gen_expr_ /*lhs*/,
                                              expr.promote() /*rhs*/);
                break;

            case optype::op_divide:
                result = Apply::make_div2_f64(this->gen_expr_ /*lhs*/,
                                              expr.promote() /*rhs*/);
                break;

            case optype::n_optype:
                /* unreachable */
                assert(false);
            }

            assert(result.get());

            /* this expression complete.. */
            std::unique_ptr<exprstate> self = p_stack->pop_exprstate();

            /* ..but more operators could follow, so don't commit yet */
            p_stack->push_exprstate(progress_xs::make(result));
        }

        void
        progress_xs::on_symbol_token(const token_type & /*tk*/,
                                     exprstatestack * /*p_stack*/,
                                     rp<Expression> * /*p_emit_expr*/)
        {
            /* illegal input, e.g.
             *   foo bar
             */
            assert(false);
        }

        void
        progress_xs::on_typedescr(TypeDescr /*td*/,
                                  exprstatestack * /*p_stack*/,
                                  rp<Expression> * /*p_emit_expr*/)
        {
            /* unreachable */
            assert(false);
        }

        void
        progress_xs::on_colon_token(const token_type & tk,
                                    exprstatestack * /*p_stack*/)
        {
            constexpr const char * self_name = "progress_xs::on_colon";

            this->illegal_input_error(self_name, tk);
        }

        void
        progress_xs::on_semicolon_token(const token_type & tk,
                                        exprstatestack * p_stack,
                                        rp<Expression> * p_emit_expr)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            rp<Expression> expr = this->gen_expr_;

            std::unique_ptr<exprstate> self = p_stack->pop_exprstate();

            p_stack->top_exprstate().on_expr(expr,
                                             p_stack,
                                             p_emit_expr);
            /* control here on input like:
             *   (1.234;
             *
             * a. '(' sets up stack [lparen_0:expect_rhs_expression]
             *     (see exprstate::on_leftparen())
             * b. 1.234 pushes (in case operators) [lparen_0:expect_rhs_expression:expr_progress]
             *     (see exprstate::on_f64())
             * c. semicolon completes expr_progress [lparen_0:expect_rhs_expression]
             *     deliver expresssion to expect_rhs_expression.on_expr()
             *     (see exprstate::on_expr())
             * d. expr_rhs_expression forwards expression to [lparen_0]
             * e. lparen_0 advances to [lparen_1]
             * f. now deliver semicolon;  [lparen_1] rejects
             */

            p_stack->top_exprstate().on_semicolon_token(tk, p_stack, p_emit_expr);
        }

        void
        progress_xs::on_singleassign_token(const token_type & tk,
                                           exprstatestack * /*p_stack*/)
        {
            constexpr const char * self_name = "progress_xs::on_singleassign";

            this->illegal_input_error(self_name, tk);
        }

        void
        progress_xs::on_leftparen_token(const token_type & tk,
                                        exprstatestack * /*p_stack*/,
                                        rp<Expression> * /*p_emit_expr*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_leftparen";

            this->illegal_input_error(self_name, tk);
        }

         void
         progress_xs::on_rightparen_token(const token_type & tk,
                                          exprstatestack * p_stack,
                                          rp<Expression> * p_emit_expr)
         {
             constexpr bool c_debug_flag = true;
             scope log(XO_DEBUG(c_debug_flag));

             constexpr const char * self_name = "progress_xs::on_rightparen";

             /* stack may be something like:
              *
              *   lparen_0
              *   expect_rhs_expression
              *   expr_progress
              *                   <-- rightparen
              *
              * 1. rightparen completes expression-in-progress
              * 2. rightparen must then match innermost waiting lparen_0
              */

             /* right paren confirms stack expression */
             rp<Expression> expr = this->gen_expr_;

             std::unique_ptr<exprstate> self = p_stack->pop_exprstate();

             if (p_stack->empty()) {
                 throw std::runtime_error(tostr(self_name,
                                                ": expected non-empty parsing stack"));
             }

             log && log(xtag("stack", p_stack));

             p_stack->top_exprstate().on_expr(expr, p_stack, p_emit_expr);

             /* now deliver rightparen */
             p_stack->top_exprstate().on_rightparen_token(tk, p_stack, p_emit_expr);

         }

        void
        progress_xs::on_operator_token(const token_type & tk,
                                       exprstatestack * p_stack,
                                       rp<Expression> * p_emit_expr)
        {
            constexpr const char * c_self_name = "progress_xs::on_operator_token";

            if (op_type_ == optype::invalid) {
                switch(tk.tk_type()) {
                case tokentype::tk_plus:
                    this->op_type_ = optype::op_add;
                    break;
                case tokentype::tk_minus:
                    this->op_type_ = optype::op_subtract;
                    break;
                case tokentype::tk_star:
                    this->op_type_ = optype::op_multiply;
                    break;
                case tokentype::tk_slash:
                    this->op_type_ = optype::op_divide;
                    break;
                default:
                    /* unreachable */
                    assert(false);
                    exprstate::on_operator_token(tk, p_stack, p_emit_expr);
                }

                /* infix operator must be followed by non-empty expression */
                p_stack->push_exprstate(expect_expr_xs::expect_rhs_expression());
            } else {
                throw std::runtime_error(tostr(c_self_name,
                                               ": expected expression following operator",
                                               xtag("tk", tk)));
            }
        }

        void
        progress_xs::on_f64_token(const token_type & tk,
                                  exprstatestack * p_stack,
                                  rp<Expression> * p_emit_expr)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "progress_xs::on_f64";

            if (this->op_type_ == optype::invalid) {
                this->illegal_input_error(self_name, tk);
            } else {
                assert(false);
                exprstate::on_f64_token(tk, p_stack, p_emit_expr);
            }
        }

        void
        progress_xs::print(std::ostream & os) const {
            os << "<progress_xs"
               << xtag("type", exs_type_);
            if (gen_expr_)
                os << xtag("gen_expr", gen_expr_);
            os << ">";
        }


    } /*namespace scm*/
} /*namespace xo*/


/* end progress_xs.cpp */
