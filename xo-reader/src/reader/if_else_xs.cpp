/** @file if_else_xs.cpp
 *
 *  author: Roland Conybeare, Jul 2025
 **/

#include "if_else_xs.hpp"
//#include "exprstatestack.hpp"
#include "parserstatemachine.hpp"
#include "expect_expr_xs.hpp"
#include "xo/indentlog/print/ppdetail_atomic.hpp"

namespace xo {
    namespace scm {
        // ----- ifexprstatetype -----

        const char *
        ifexprstatetype_descr(ifexprstatetype x) {
            switch (x) {
            case ifexprstatetype::invalid: return "invalid";
            case ifexprstatetype::if_0: return "if_0";
            case ifexprstatetype::if_1: return "if_1";
            case ifexprstatetype::if_2: return "if_2";
            case ifexprstatetype::if_3: return "if_3";
            case ifexprstatetype::if_4: return "if_4";
            case ifexprstatetype::if_5: return "if_5";
            case ifexprstatetype::if_6: return "if_6";
            case ifexprstatetype::n_ifexprstatetype: break;
            }

            return "???ifexprstatetype";
        }

        std::ostream &
        operator<<(std::ostream & os, ifexprstatetype x) {
            os << ifexprstatetype_descr(x);
            return os;
        }

        // ----- if_else_xs -----

        std::unique_ptr<if_else_xs>
        if_else_xs::make() {
            return std::make_unique<if_else_xs>(if_else_xs(IfExprAccess::make_empty()));
        }

        void
        if_else_xs::start(parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            p_psm->push_exprstate(if_else_xs::make());
            p_psm->top_exprstate().on_if_token(token_type::if_token(), p_psm);
        }

        if_else_xs::if_else_xs(rp<IfExprAccess> if_expr)
            : exprstate(exprstatetype::ifexpr),
              ifxs_type_{ifexprstatetype::if_0},
              if_expr_{std::move(if_expr)}
        {}

        const char *
        if_else_xs::get_expect_str() const
        {
            /**
             *   if test-expr then then-expr else else-expr ;
             *  ^  ^         ^    ^         ^    ^         ^
             *  |  |         |    |         |    |         |
             *  |  if_1      if_2 if_3      if_4 if_5      if_6
             *  if_0
             *
             *  if_0 --on_if_token()--> if_1
             *  if_1 --on_expr()--> if_2
             *  if_2 --on_then_token()--> if_3
             *  if_3 --on_expr()--> if_4
             *  if_4 --on_else_token()--> if_5
             *       --on_semicolon_token()--> (done)
             *  if_5 --on_expr()-->if_6
             *  if_6 --on_semicolon_token()--> (done)
             **/
            switch (this->ifxs_type_) {
            case ifexprstatetype::invalid:
            case ifexprstatetype::if_0:
            case ifexprstatetype::n_ifexprstatetype:
                assert(false); // unreachable
                return nullptr;
            case ifexprstatetype::if_1:
                return "expression";
            case ifexprstatetype::if_2:
                return "then";
            case ifexprstatetype::if_3:
                return "expression";
            case ifexprstatetype::if_4:
                return "else|semicolon";
            case ifexprstatetype::if_5:
                return "expression";
            case ifexprstatetype::if_6:
                return "semicolon";
            }

            return "?expect";
        }

        void
        if_else_xs::on_if_token(const token_type & tk,
                                parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            log && log("ifxs_type", ifxs_type_);

            if (this->ifxs_type_ == ifexprstatetype::if_0) {
                this->ifxs_type_ = ifexprstatetype::if_1;

                expect_expr_xs::start(p_psm);
                return;
            }

            constexpr const char * c_self_name = "if_else_xs::on_if_token";
            const char * exp = this->get_expect_str();

            this->illegal_input_on_token(c_self_name, tk, exp, p_psm);
        }

        void
        if_else_xs::on_then_token(const token_type & tk,
                                  parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            log && log("ifxs_type", ifxs_type_);

            if (this->ifxs_type_ == ifexprstatetype::if_2) {
                this->ifxs_type_ = ifexprstatetype::if_3;

                expect_expr_xs::start(p_psm);
                return;
            }

            constexpr const char * c_self_name = "if_else_xs::on_then_token";
            const char * exp = this->get_expect_str();

            this->illegal_input_on_token(c_self_name, tk, exp, p_psm);
        }

        void
        if_else_xs::on_else_token(const token_type & tk,
                                  parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            log && log("ifxs_type", ifxs_type_);

            if (this->ifxs_type_ == ifexprstatetype::if_4) {
                this->ifxs_type_ = ifexprstatetype::if_5;

                expect_expr_xs::start(p_psm);
                return;
            }

            constexpr const char * c_self_name = "if_else_xs::on_else_token";
            const char * exp = this->get_expect_str();

            this->illegal_input_on_token(c_self_name, tk, exp, p_psm);
        }

        void
        if_else_xs::finish_and_continue(parserstatemachine * p_psm)
        {
            rp<IfExprAccess> if_expr = this->if_expr_;
            std::unique_ptr<exprstate> self = p_psm->pop_exprstate();

            if (this->ifxs_type_ == ifexprstatetype::if_4) {
                /* if no else-branch, then if-expr can't have valuetype */
                if_expr->assign_valuetype(nullptr);
            }

            p_psm->top_exprstate().on_expr(if_expr, p_psm);
        }

        void
        if_else_xs::on_rightbrace_token(const token_type & tk,
                                        parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            this->finish_and_continue(p_psm);
            p_psm->on_rightbrace_token(tk);
        }

        void
        if_else_xs::on_semicolon_token(const token_type & tk,
                                       parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            log && log("ifxs_type", ifxs_type_);

            const char * c_self_name = "if_else_xs::on_semicolon_token";

            switch (this->ifxs_type_) {
            case ifexprstatetype::invalid:
            case ifexprstatetype::if_0:
            case ifexprstatetype::n_ifexprstatetype:
                // unreachable
                assert(false);
                break;

            case ifexprstatetype::if_1:
            case ifexprstatetype::if_2:
            case ifexprstatetype::if_3:
            case ifexprstatetype::if_5:
                this->illegal_input_on_token(c_self_name, tk, get_expect_str(), p_psm);
                break;
            case ifexprstatetype::if_4:
            case ifexprstatetype::if_6: {
                this->finish_and_continue(p_psm);
                break;
            }
            }
        }

        void
        if_else_xs::on_expr(bp<Expression> expr,
                            parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            log && log(xtag("ifxs_type", ifxs_type_));

            switch (this->ifxs_type_) {
            case ifexprstatetype::invalid:
            case ifexprstatetype::if_0:
            case ifexprstatetype::n_ifexprstatetype:
                assert(false); // unreachable
                return;
            case ifexprstatetype::if_1:
                if_expr_->assign_test(expr.promote());
                ifxs_type_ = ifexprstatetype::if_2;
                return;
            case ifexprstatetype::if_2:
                /** error: expecting 'then' **/
                break;
            case ifexprstatetype::if_3:
                if_expr_->assign_when_true(expr.promote());
                ifxs_type_ = ifexprstatetype::if_4;
                return;
            case ifexprstatetype::if_4:
                /** error: expecting 'else' or ';' **/
                break;
            case ifexprstatetype::if_5:
                if_expr_->assign_when_false(expr.promote());
                ifxs_type_ = ifexprstatetype::if_6;
                return;
            case ifexprstatetype::if_6:
                /** error: expecting ';' **/
                break;
            }

            constexpr const char* c_self_name = "if_else_xs::on_expr";
            const char * exp = get_expect_str();

            this->illegal_input_on_expr(c_self_name, expr, exp, p_psm);
        }

        void
        if_else_xs::on_expr_with_semicolon(bp<Expression> expr,
                                           parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));


            log && log(xtag("ifxs_type", ifxs_type_));

            this->on_expr(expr, p_psm);
            this->on_semicolon_token(token_type::semicolon(), p_psm);
        }

        void
        if_else_xs::print(std::ostream & os) const {
            os << "<if_else_xs"
               << xtag("this", (void*)this)
               << xtag("ifxs_type", ifxs_type_)
               << ">";
        }

    } /*namespace scm*/
} /*namespace xo*/
