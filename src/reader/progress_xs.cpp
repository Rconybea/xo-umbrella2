/* @file progress_xs.cpp */

#include "progress_xs.hpp"
#include "expect_expr_xs.hpp"
#include "parserstatemachine.hpp"
#include "xo/expression/Apply.hpp"

namespace xo {
    using xo::ast::Expression;
    using xo::ast::Apply;

    namespace scm {
        const char *
        optype_descr(optype x) {
            switch (x) {
            case optype::invalid:
                return "?optype";
            case optype::op_add:
                return "op+";
            case optype::op_subtract:
                return "op-";
            case optype::op_multiply:
                return "op*";
            case optype::op_divide:
                return "op/";
            case optype::n_optype:
                break;
            }
            return "???";
        }

        int
        precedence(optype x) {
            switch (x) {
            case optype::invalid:
            case optype::n_optype:
                return 0;

            case optype::op_add:
            case optype::op_subtract:
                return 1;

            case optype::op_multiply:
            case optype::op_divide:
                return 2;
            }

            return 0;
        }

        std::unique_ptr<progress_xs>
        progress_xs::make(rp<Expression> valex, optype op) {
            return std::make_unique<progress_xs>(progress_xs(std::move(valex), op));
        }

        void
        progress_xs::start(rp<Expression> valex, optype op, exprstatestack * p_stack) {
            p_stack->push_exprstate(progress_xs::make(valex, op));
        }

        void
        progress_xs::start(rp<Expression> valex, exprstatestack * p_stack) {
            p_stack->push_exprstate(progress_xs::make(valex));
        }

        progress_xs::progress_xs(rp<Expression> valex, optype op)
            : exprstate(exprstatetype::expr_progress),
              lhs_{std::move(valex)},
              op_type_{op}
        {}

        bool
        progress_xs::admits_f64() const { return false; }

        void
        progress_xs::on_def_token(const token_type & tk,
                                  parserstatemachine * /*p_stack*/)
        {
            constexpr const char * self_name = "progress_xs::on_def";

            /* nothing here - admits_definition unconditionally false */
            this->illegal_input_error(self_name, tk) ;
        }

        rp<Expression>
        progress_xs::assemble_expr() {
            /* need to defer building Apply incase expr followed by higher-precedence operator:
             * consider input like
             *   3.14 + 2.0 * ...
             */

            constexpr const char * c_self_name = "progress_xs::assemble_expr";

            if ((op_type_ != optype::invalid) && (rhs_.get() == nullptr)) {
                throw std::runtime_error(tostr(c_self_name,
                                               ": expected expr on rhs of operator",
                                               xtag("lhs", lhs_),
                                               xtag("op", op_type_)));
            }

            /* consecutive expressions not legal, e.g:
             *   3.14 6.28
             * but expressions surrounding an infix operators is:
             *   3.14 / 6.28
             */
            switch (op_type_) {
            case optype::invalid:
                return this->lhs_;

            case optype::op_add:
                return Apply::make_add2_f64(this->lhs_,
                                            this->rhs_);

            case optype::op_subtract:
                return Apply::make_sub2_f64(this->lhs_,
                                              this->rhs_);

            case optype::op_multiply:
                return Apply::make_mul2_f64(this->lhs_,
                                              this->rhs_);

            case optype::op_divide:
                return Apply::make_div2_f64(this->lhs_,
                                            this->rhs_);

            case optype::n_optype:
                /* unreachable */
                assert(false);
                return nullptr;
            }

            return nullptr;
        }

        void
        progress_xs::on_expr(ref::brw<Expression> expr,
                             parserstatemachine * /*p_psm*/)
        {
            /* note: previous token probably an operator,
             *       handled from progress_xs::on_operator_token(),
             *       which pushes expect_expr_xs::expect_rhs_expression()
             */

            constexpr const char * c_self_name = "progress_xs::on_expr";


            if (op_type_ == optype::invalid) {
                throw std::runtime_error(tostr(c_self_name,
                                               ": consecutive unseparated exprs not legal"));
            }

#ifdef NOT_QUITE
            assert(result.get());

            /* this expression complete.. */
            std::unique_ptr<exprstate> self = p_stack->pop_exprstate();

            /* ..but more operators could follow, so don't commit yet */
            p_stack->push_exprstate(progress_xs::make(result));
#endif

            this->rhs_ = expr.promote();
        }

        void
        progress_xs::on_symbol_token(const token_type & /*tk*/,
                                     parserstatemachine * /*p_psm*/)
        {
            /* illegal input, e.g.
             *   foo bar
             */
            assert(false);
        }

        void
        progress_xs::on_typedescr(TypeDescr /*td*/,
                                  parserstatemachine * /*p_psm*/)
        {
            /* unreachable */
            assert(false);
        }

        void
        progress_xs::on_colon_token(const token_type & tk,
                                    parserstatemachine * /*p_psm*/)
        {
            constexpr const char * self_name = "progress_xs::on_colon";

            this->illegal_input_error(self_name, tk);
        }

        void
        progress_xs::on_semicolon_token(const token_type & tk,
                                        parserstatemachine * p_psm)
        {
            /* note: implementation parllels .on_rightparen_token() */

            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            auto p_stack = p_psm->p_stack_;

            rp<Expression> expr = this->assemble_expr();

            std::unique_ptr<exprstate> self = p_stack->pop_exprstate();

            p_stack->top_exprstate().on_expr(expr, p_psm);

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

            p_stack->top_exprstate().on_semicolon_token(tk, p_psm);
        }

        void
        progress_xs::on_singleassign_token(const token_type & tk,
                                           parserstatemachine * /*p_psm*/)
        {
            constexpr const char * self_name = "progress_xs::on_singleassign";

            this->illegal_input_error(self_name, tk);
        }

        void
        progress_xs::on_leftparen_token(const token_type & tk,
                                        parserstatemachine * /*p_psm*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_leftparen";

            this->illegal_input_error(self_name, tk);
        }

         void
         progress_xs::on_rightparen_token(const token_type & tk,
                                          parserstatemachine * p_psm)
         {
             /* note: implementation parallels .on_semicolon_token() */


             constexpr bool c_debug_flag = true;
             scope log(XO_DEBUG(c_debug_flag));

             constexpr const char * self_name = "progress_xs::on_rightparen";

             auto p_stack = p_psm->p_stack_;

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
             rp<Expression> expr = this->assemble_expr();

             std::unique_ptr<exprstate> self = p_stack->pop_exprstate();

             if (p_stack->empty()) {
                 throw std::runtime_error(tostr(self_name,
                                                ": expected non-empty parsing stack"));
             }

             log && log(xtag("stack", p_stack));

             p_stack->top_exprstate().on_expr(expr, p_psm);

             /* now deliver rightparen */
             p_stack->top_exprstate().on_rightparen_token(tk, p_psm);
         }

        namespace {
            optype
            tk2op(const tokentype & tktype) {
                switch (tktype) {
                case tokentype::tk_plus:
                    return optype::op_add;
                case tokentype::tk_minus:
                    return optype::op_subtract;
                case tokentype::tk_star:
                    return optype::op_multiply;
                case tokentype::tk_slash:
                    return optype::op_divide;
                default:
                    assert(false);
                    return optype::invalid;
                }
                return optype::invalid;
            }
        }

        void
        progress_xs::on_operator_token(const token_type & tk,
                                       parserstatemachine * p_psm)
        {
            constexpr const char * c_self_name = "progress_xs::on_operator_token";

            auto p_stack = p_psm->p_stack_;

            if (op_type_ == optype::invalid) {
                this->op_type_ = tk2op(tk.tk_type());

                /* infix operator must be followed by non-empty expression */
                expect_expr_xs::start(p_stack);
            } else if (rhs_) {
                /* already have complete expression stashed.
                 * behavior depends on operator precedence for tk with stored operator
                 * this->op_type_
                 */
                optype op2 = tk2op(tk.tk_type());

                if (precedence(op2) <= precedence(this->op_type_)) {
                    /* e.g.
                     *   6.2 * 4.9 + ...
                     *
                     * in stack:
                     *   1. progress_xs lhs=6.2, op=*, rhs=4.9
                     *
                     * out stack
                     *   1. progress_xs lhs=apply(*,6.2,4.9), op=+
                     */

                    /* 1. instantiate expression for *this */
                    auto expr = this->assemble_expr();

                    /* 2. remove from stack */
                    std::unique_ptr<exprstate> self  = p_stack->pop_exprstate();

                    /* 3. replace with new progress_xs: */
                    progress_xs::start(expr, op2, p_stack);

                    /* infix operator must be followed by non-empty expression */
                    expect_expr_xs::start(p_stack);
                } else {
                    /* e.g.
                     *   6.2 + 4.9 * ...
                     *
                     * in stack:
                     *   1. progress_xs lhs=6.2, op=+, rhs=4.9
                     *
                     * out stack:
                     *   1. progress_xs lhs=6.2, op=+
                     *   2. expect_rhs_expression
                     *   3. progress_xs lhs=4.9, op=*
                     *   4. expect_rhs_expression
                     */

                    std::unique_ptr<exprstate> self = p_stack->pop_exprstate();

                    /* 1. replace with nested incomplete infix exprs */
                    progress_xs::start(lhs_, op_type_, p_stack);
                    expect_expr_xs::start(p_stack);
                    progress_xs::start(rhs_, op2, p_stack);
                    expect_expr_xs::start(p_stack);
                }

            } else {
                throw std::runtime_error(tostr(c_self_name,
                                               ": expected expression following operator",
                                               xtag("tk", tk)));
            }
        }

        void
        progress_xs::on_f64_token(const token_type & tk,
                                  parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "progress_xs::on_f64";

            if (this->op_type_ == optype::invalid) {
                this->illegal_input_error(self_name, tk);
            } else {
                exprstate::on_f64_token(tk, p_psm);
            }
        }

        void
        progress_xs::print(std::ostream & os) const {
            os << "<progress_xs"
               << xtag("type", exs_type_);
            if (lhs_)
                os << xtag("lhs", lhs_);
            if (op_type_ != optype::invalid)
                os << xtag("op", op_type_);
            if (rhs_)
                os << xtag("rhs", rhs_);
            os << ">";
        }


    } /*namespace scm*/
} /*namespace xo*/


/* end progress_xs.cpp */
