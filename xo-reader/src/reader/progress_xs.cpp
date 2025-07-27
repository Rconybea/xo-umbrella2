/* @file progress_xs.cpp */

#include "progress_xs.hpp"
#include "apply_xs.hpp"
#include "exprstatestack.hpp"
#include "expect_expr_xs.hpp"
#include "parserstatemachine.hpp"
#include "pretty_exprstatestack.hpp"
#include "xo/expression/AssignExpr.hpp"
#include "xo/expression/Apply.hpp"
#include "xo/expression/pretty_expression.hpp"

namespace xo {
    using xo::ast::Expression;
    using xo::ast::AssignExpr;
    using xo::ast::Variable;
    using xo::ast::Apply;

    namespace scm {
        const char *
        optype_descr(optype x) {
            switch (x) {
            case optype::invalid:
                return "?optype";
            case optype::op_assign:
                return "op:=";
            case optype::op_less:
                return "op<";
            case optype::op_less_equal:
                return "op<=";
            case optype::op_equal:
                return "op==";
            case optype::op_not_equal:
                return "op!=";
            case optype::op_great:
                return "op>";
            case optype::op_great_equal:
                return "op>=";
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

            case optype::op_assign:
                return 1;

            case optype::op_less:
            case optype::op_less_equal:
            case optype::op_equal:
            case optype::op_not_equal:
            case optype::op_great:
            case optype::op_great_equal:
                return 2;

            case optype::op_add:
            case optype::op_subtract:
                return 3;

            case optype::op_multiply:
            case optype::op_divide:
                return 4;
            }

            return 0;
        }

        std::unique_ptr<progress_xs>
        progress_xs::make(rp<Expression> valex, optype op) {
            return std::make_unique<progress_xs>(progress_xs(std::move(valex), op));
        }

        void
        progress_xs::start(rp<Expression> valex, optype op, parserstatemachine * p_psm) {
            p_psm->push_exprstate(progress_xs::make(valex, op));
        }

        void
        progress_xs::start(rp<Expression> valex, parserstatemachine * p_psm) {
            p_psm->push_exprstate(progress_xs::make(valex));
        }

        progress_xs::progress_xs(rp<Expression> valex, optype op)
            : exprstate(exprstatetype::expr_progress),
              lhs_{std::move(valex)},
              op_type_{op}
        {}

        bool
        progress_xs::admits_f64() const { return false; }

        const char *
        progress_xs::get_expect_str() const {
            if (op_type_ == optype::invalid) {
                return "oper|semicolon|rightparen";
            } else {
                return "expr|leftparen";
            }
        }

        void
        progress_xs::on_def_token(const token_type & tk,
                                  parserstatemachine * p_psm)
        {
            constexpr const char * c_self_name = "progress_xs::on_def";
            const char * exp = get_expect_str();

            /* nothing here - admits_definition unconditionally false */
            this->illegal_input_on_token(c_self_name, tk, exp, p_psm);
        }

        void
        progress_xs::apply_type_error(const char * self_name,
                                      optype op,
                                      bp<Expression> expr1,
                                      bp<Expression> expr2,
                                      parserstatemachine * p_psm) const
        {
            std::string errmsg = tostr("incompatible argument types T1,T2 to op",
                                       xtag("op", op),
                                       xtag("T1", expr1->valuetype()),
                                       xtag("T2", expr2->valuetype()));

            p_psm->on_error(self_name, std::move(errmsg));
        }

        rp<Expression>
        progress_xs::assemble_expr(parserstatemachine * p_psm) {
            /* need to defer building Apply incase expr followed by higher-precedence operator:
             * consider input like
             *   3.14 + 2.0 * ...
             */

            constexpr const char * c_self_name = "progress_xs::assemble_expr";

            if ((op_type_ != optype::invalid) && (rhs_.get() == nullptr)) {
                std::string errmsg = tostr("expected expression on rhs of operator op",
                                           xtag("lhs", lhs_),
                                           xtag("op", op_type_));

                p_psm->on_error(c_self_name, errmsg);
            }

            /* consecutive expressions not legal, e.g:
             *   3.14 6.28
             * but expressions surrounding an infix operators is:
             *   3.14 / 6.28
             */
            switch (op_type_) {
            case optype::invalid:
                return this->lhs_;

            case optype::op_assign:
            {
                bp<Variable> lhs = Variable::from(this->lhs_);

                if (!lhs) {
                    throw std::runtime_error
                        (tostr("progress_xs::assemble_expr",
                               " expect variable on lhs of assignment operator :=",
                               xtag("lhs", lhs_),
                               xtag("rhs", rhs_)));
                }

                return AssignExpr::make(lhs.promote(),
                                        this->rhs_);
            }

            case optype::op_equal:
                if (lhs_->valuetype()->is_i64() && rhs_->valuetype()->is_i64()) {
                    return Apply::make_cmp_eq_i64(lhs_, rhs_);
                } else {
                    this->apply_type_error(c_self_name,
                                           op_type_, lhs_, rhs_, p_psm);
                    return nullptr;
                }
                break;

            case optype::op_not_equal:
                assert(false);

            case optype::op_less:
                // TODO: floating-point less-than

                if (lhs_->valuetype()->is_i64() && rhs_->valuetype()->is_i64()) {
                    return Apply::make_cmp_lt_i64(lhs_, rhs_);
                } else {
                    this->apply_type_error(c_self_name,
                                           op_type_, lhs_, rhs_, p_psm);
                    return nullptr;
                }
                break;

            case optype::op_less_equal:
                if (lhs_->valuetype()->is_i64() && rhs_->valuetype()->is_i64()) {
                    return Apply::make_cmp_le_i64(lhs_, rhs_);
                } else {
                    this->apply_type_error(c_self_name,
                                           op_type_, lhs_, rhs_, p_psm);
                    return nullptr;
                }
                break;

            case optype::op_great:
                if (lhs_->valuetype()->is_i64() && rhs_->valuetype()->is_i64()) {
                    return Apply::make_cmp_gt_i64(lhs_, rhs_);
                } else {
                    this->apply_type_error(c_self_name,
                                           op_type_, lhs_, rhs_, p_psm);
                    return nullptr;
                }
                break;

            case optype::op_great_equal:
                // TODO: upconvert integer->double
                if (lhs_->valuetype()->is_i64() && rhs_->valuetype()->is_i64()) {
                    return Apply::make_cmp_ge_i64(lhs_, rhs_);
                } else {
                    this->apply_type_error(c_self_name,
                                           op_type_, lhs_, rhs_, p_psm);
                    return nullptr;
                }

                assert(false);

            case optype::op_add:
                // TODO: upconvert integer->double
                if (lhs_->valuetype()->is_i64() && rhs_->valuetype()->is_i64()) {
                    return Apply::make_add2_i64(lhs_, rhs_);
                } else if (lhs_->valuetype()->is_f64() && rhs_->valuetype()->is_f64()) {
                    return Apply::make_add2_f64(lhs_, rhs_);
                } else {
                    this->apply_type_error(c_self_name,
                                           op_type_, lhs_, rhs_, p_psm);
                    return nullptr;
                }
                break;
            case optype::op_subtract:
                // TODO: upconvert integer->double
                if (lhs_->valuetype()->is_i64() && rhs_->valuetype()->is_i64()) {
                    return Apply::make_sub2_i64(lhs_, rhs_);
                } else if (lhs_->valuetype()->is_f64() && rhs_->valuetype()->is_f64()) {
                    return Apply::make_sub2_f64(lhs_, rhs_);
                } else {
                    this->apply_type_error(c_self_name,
                                           op_type_, lhs_, rhs_, p_psm);
                    return nullptr;
                }
                break;

            case optype::op_multiply:
                // TODO: upconvert integer->double
                if (lhs_->valuetype()->is_i64() && rhs_->valuetype()->is_i64()) {
                    return Apply::make_mul2_i64(lhs_, rhs_);
                } else if (lhs_->valuetype()->is_f64() && rhs_->valuetype()->is_f64()) {
                    return Apply::make_mul2_f64(lhs_, rhs_);
                } else {
                    this->apply_type_error(c_self_name,
                                           op_type_, lhs_, rhs_, p_psm);
                    return nullptr;
                }

                break;

            case optype::op_divide:
                // TODO: upconvert integer->double
                if (lhs_->valuetype()->is_i64() && rhs_->valuetype()->is_i64()) {
                    return Apply::make_div2_i64(lhs_, rhs_);
                } else if (lhs_->valuetype()->is_f64() && rhs_->valuetype()->is_f64()) {
                    return Apply::make_div2_f64(lhs_, rhs_);
                } else {
                    this->apply_type_error(c_self_name,
                                           op_type_, lhs_, rhs_, p_psm);
                    return nullptr;
                }
                break;

            case optype::n_optype:
                /* unreachable */
                assert(false);
                return nullptr;
            }

            return nullptr;
        }

        void
        progress_xs::on_expr(bp<Expression> expr,
                             parserstatemachine * p_psm)
        {
            /* note: previous token probably an operator,
             *       handled from progress_xs::on_operator_token(),
             *       which pushes expect_expr_xs::expect_rhs_expression()
             */

            constexpr const char * c_self_name = "progress_xs::on_expr";
            const char * exp = get_expect_str();

            if (op_type_ == optype::invalid) {
                this->illegal_input_on_expr(c_self_name, expr, exp, p_psm);
            }

#ifdef NOT_QUITE
            assert(result.get());

            /* this expression complete.. */
            std::unique_ptr<exprstate> self = p_psm->pop_exprstate();

            /* ..but more operators could follow, so don't commit yet */
            p_stack->push_exprstate(progress_xs::make(result));
#endif

            this->rhs_ = expr.promote();
        }

        void
        progress_xs::on_expr_with_semicolon(bp<Expression> expr,
                                            parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("lhs", lhs_), xtag("op", op_type_), xtag("expr", expr));

            constexpr const char * c_self_name = "progress_xs::on_expr_with_semicolon";
            const char * exp = get_expect_str();

            if (op_type_ == optype::invalid) {
                this->illegal_input_on_expr(c_self_name, expr, exp, p_psm);
            }

            this->rhs_ = expr.promote();

            // FORBIDDEN, because .on_semicolon_token() destroys *this before returning
            //   this->on_semicolon_token(token_type::semicolon(), p_psm);
            // INSTEAD, spell out the body

            rp<Expression> expr2 = this->assemble_expr(p_psm);

            if (expr2) {
                std::unique_ptr<exprstate> self = p_psm->pop_exprstate();

                p_psm->on_expr_with_semicolon(expr2);
            }
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
                                    parserstatemachine * p_psm)
        {
            constexpr const char * c_self_name = "progress_xs::on_colon";
            const char * exp = get_expect_str();

            this->illegal_input_on_token(c_self_name, tk, exp, p_psm);
        }

        void
        progress_xs::on_semicolon_token(const token_type & /*tk*/,
                                        parserstatemachine * p_psm)
        {
            /* note: implementation parallels .on_rightparen_token() */

            scope log(XO_DEBUG(p_psm->debug_flag()));

            rp<Expression> expr = this->assemble_expr(p_psm);

            log && log(xtag("assembled-expr", expr));

            std::unique_ptr<exprstate> self = p_psm->pop_exprstate();

            p_psm->on_expr_with_semicolon(expr);

            /* control here on input like:
             *   (1.234;
             *
             * a. '(' sets up stack [lparen_0:expect_rhs_expression]
             *     (see exprstate::on_leftparen())
             * b. 1.234 pushes (in case operators) [lparen_0:expect_rhs_expression:expr_progress]
             *     (see exprstate::on_f64())
             * c. semicolon completes expr_progress [lparen_0:expect_rhs_expression]
             *     deliver expresssion to expect_rhs_expression.on_expr_with_semicolon()
             *     (see exprstate::on_expr_with_semicolon())
             * d. expr_rhs_expression forwards expression to [lparen_0]
             * e. lparen_0 would advance to [lparen_1],  but rejects semicolon
             */
        }

        void
        progress_xs::on_singleassign_token(const token_type & tk,
                                           parserstatemachine * p_psm)
        {
            constexpr const char * c_self_name = "progress_xs::on_singleassign";
            const char * exp = get_expect_str();

            this->illegal_input_on_token(c_self_name, tk, exp, p_psm);
        }

        void
        progress_xs::on_leftparen_token(const token_type & tk,
                                        parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            /* input like:
             *   'foo(' -> expect function call.  might continue 'foo(a,b,c)'
             *   'foo+(' -> expect parenthesized expression.  might continue 'foo+(bar/2)'
             */

            if (op_type_ == optype::invalid) {
                /* start function call */
                assert(rhs_.get() == nullptr);

                /* unwind this progress_xs + replace with function call */

                rp<Expression> fn_expr = lhs_;
                std::unique_ptr<exprstate> self = p_psm->pop_exprstate();

                apply_xs::start(fn_expr, p_psm);
                return;
            }

            constexpr const char * c_self_name = "exprstate::on_leftparen";
            const char * exp = get_expect_str();

            this->illegal_input_on_token(c_self_name, tk, exp, p_psm);
        }

         void
         progress_xs::on_rightparen_token(const token_type & tk,
                                          parserstatemachine * p_psm)
         {
             /* note: implementation parallels .on_semicolon_token() */

             scope log(XO_DEBUG(p_psm->debug_flag()));

             constexpr const char * self_name = "progress_xs::on_rightparen";

             auto & xs_stack = p_psm->xs_stack_;

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
             rp<Expression> expr = this->assemble_expr(p_psm);

             std::unique_ptr<exprstate> self = p_psm->pop_exprstate();

             if (xs_stack.empty()) {
                 throw std::runtime_error(tostr(self_name,
                                                ": expected non-empty parsing stack"));
             }

             log && log(xtag("stack", &xs_stack));

             p_psm->top_exprstate().on_expr(expr, p_psm);

             /* now deliver rightparen */
             p_psm->top_exprstate().on_rightparen_token(tk, p_psm);
         }

        void
        progress_xs::on_then_token(const token_type & tk,
                                   parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            rp<Expression> expr = this->assemble_expr(p_psm);

            log && log(xtag("assembled-expr", expr));

            std::unique_ptr<exprstate> self = p_psm->pop_exprstate();

            p_psm->on_expr(expr);
            p_psm->on_then_token(tk);

            /* control here on input like:
             *
             *   if a > b then..
             *
             */
        }

        void
        progress_xs::on_else_token(const token_type & tk,
                                   parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            rp<Expression> expr = this->assemble_expr(p_psm);

            log && log(xtag("assembled-expr", expr));

            std::unique_ptr<exprstate> self = p_psm->pop_exprstate();

            p_psm->on_expr(expr);
            p_psm->on_else_token(tk);

            /* control here on input like:
             *
             *   if a > b then c else..
             */
        }

        void
        progress_xs::on_rightbrace_token(const token_type & tk,
                                         parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            rp<Expression> expr = this->assemble_expr(p_psm);

            log && log(xtag("assembled-expr", expr));

            std::unique_ptr<exprstate> self = p_psm->pop_exprstate();

            p_psm->on_expr(expr);
            p_psm->on_rightbrace_token(tk);

            /* control here on input like:
             *
             *   { n * n }
             */
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
                case tokentype::tk_cmpeq:
                    return optype::op_equal;
                case tokentype::tk_cmpne:
                    return optype::op_not_equal;
                case tokentype::tk_leftangle:
                    return optype::op_less;
                case tokentype::tk_lessequal:
                    return optype::op_less_equal;
                case tokentype::tk_rightangle:
                    return optype::op_great;
                case tokentype::tk_greatequal:
                    return optype::op_great_equal;
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
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * c_self_name = "progress_xs::on_operator_token";

            if (op_type_ == optype::invalid) {
                this->op_type_ = tk2op(tk.tk_type());

                /* infix operator must be followed by non-empty expression */
                expect_expr_xs::start(p_psm);
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
                    auto expr = this->assemble_expr(p_psm);

                    /* 2. remove from stack */
                    std::unique_ptr<exprstate> self  = p_psm->pop_exprstate();

                    /* 3. replace with new progress_xs: */
                    progress_xs::start(expr, op2, p_psm);

                    /* infix operator must be followed by non-empty expression */
                    expect_expr_xs::start(p_psm);
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

                    std::unique_ptr<exprstate> self = p_psm->pop_exprstate();

                    /* 1. replace with nested incomplete infix exprs */
                    progress_xs::start(lhs_, op_type_, p_psm);
                    expect_expr_xs::start(p_psm);
                    progress_xs::start(rhs_, op2, p_psm);
                    expect_expr_xs::start(p_psm);
                }

            } else {
                throw std::runtime_error(tostr(c_self_name,
                                               ": expected expression following operator",
                                               xtag("tk", tk)));
            }
        }

        void
        progress_xs::on_bool_token(const token_type & tk,
                                   parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            constexpr const char * c_self_name = "progress_xs::on_bool_token";
            const char * exp = get_expect_str();

            if (this->op_type_ == optype::invalid) {
                this->illegal_input_on_token(c_self_name, tk, exp, p_psm);
            } else {
                exprstate::on_bool_token(tk, p_psm);
            }
        }

        void
        progress_xs::on_i64_token(const token_type & tk,
                                  parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * c_self_name = "progress_xs::on_i64_token";
            const char * exp = get_expect_str();

            if (this->op_type_ == optype::invalid) {
                this->illegal_input_on_token(c_self_name, tk, exp, p_psm);
            } else {
                exprstate::on_i64_token(tk, p_psm);
            }
        }

        void
        progress_xs::on_f64_token(const token_type & tk,
                                  parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * c_self_name = "progress_xs::on_f64_token";
            const char * exp = get_expect_str();

            if (this->op_type_ == optype::invalid) {
                this->illegal_input_on_token(c_self_name, tk, exp, p_psm);
            } else {
                exprstate::on_f64_token(tk, p_psm);
            }
        }

        void
        progress_xs::print(std::ostream & os) const {
            os << "<progress_xs"
               << xtag("this", (void*)this)
               << xtag("type", exs_type_);
            if (lhs_)
                os << xtag("lhs", lhs_);
            if (op_type_ != optype::invalid)
                os << xtag("op", op_type_);
            if (rhs_)
                os << xtag("rhs", rhs_);
            os << ">";
        }

        bool
        progress_xs::pretty_print(const xo::print::ppindentinfo & ppii) const
        {
            if (ppii.upto()) {
                return (ppii.pps()->print_upto("<progress_xs")
                        && (lhs_ ? ppii.pps()->print_upto(refrtag("lhs", lhs_)) : true)
                        && (op_type_ != optype::invalid ? ppii.pps()->print_upto(refrtag("op", op_type_)) : true)
                        && (rhs_ ? ppii.pps()->print_upto(refrtag("rhs", rhs_)) : true)
                        && ppii.pps()->print_upto(">"));
            } else {
                ppii.pps()->write("<progress_xs");
                if (lhs_)
                    ppii.pps()->pretty(refrtag("lhs", lhs_));
                if (op_type_ != optype::invalid)
                    ppii.pps()->pretty(refrtag("op", op_type_));
                if (rhs_)
                    ppii.pps()->pretty(refrtag("rhs", rhs_));
                ppii.pps()->write(">");
                return false;
            }
        }

    } /*namespace scm*/
} /*namespace xo*/


/* end progress_xs.cpp */
