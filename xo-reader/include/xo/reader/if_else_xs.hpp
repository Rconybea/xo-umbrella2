/** @file if_else_xs.hpp
 *
 *  author: Roland Conybeare, Jul 2025
 **/

#pragma once

#include "xo/expression/IfExpr.hpp"
#include "exprstate.hpp"
#include "xo/indentlog/print/ppdetail_atomic.hpp"

namespace xo {
    namespace scm {
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
        enum class ifexprstatetype {
            invalid = -1,

            if_0,
            if_1,
            if_2,
            if_3,
            if_4,
            if_5,
            if_6,

            n_ifexprstatetype,
        };

        extern const char * ifexprstatetype_descr(ifexprstatetype x);

        std::ostream &
        operator<<(std::ostream & os, ifexprstatetype x);

        /** @class if_else_xs
         *  @brief state to provide parsing of a conditional expression
         **/
        class if_else_xs : public exprstate {
        public:
            using IfExprAccess = xo::ast::IfExprAccess;

        public:
            if_else_xs(rp<IfExprAccess> if_expr);
            virtual ~if_else_xs() = default;

            static const if_else_xs * from(const exprstate * x) { return dynamic_cast<const if_else_xs *>(x); }

            static void start(parserstatemachine * p_psm);

            // ----- inherited from exprstate -----

            virtual const char * get_expect_str() const override;

            virtual void on_if_token(const token_type & tk,
                                     parserstatemachine * p_psm) override;
            virtual void on_then_token(const token_type & tk,
                                       parserstatemachine * p_psm) override;
            virtual void on_else_token(const token_type & tk,
                                       parserstatemachine * p_psm) override;
            virtual void on_semicolon_token(const token_type & tk,
                                            parserstatemachine * p_psm) override;
            virtual void on_rightbrace_token(const token_type & tk,
                                             parserstatemachine * p_psm) override;

            virtual void on_expr(bp<Expression> expr,
                                 parserstatemachine * p_psm) override;
            virtual void on_expr_with_semicolon(bp<Expression> expr,
                                                parserstatemachine * p_psm) override;

            virtual void print(std::ostream & os) const override;

        private:
            static std::unique_ptr<if_else_xs> make();

            /** exit this exprstate,
             *  and deliver @ref if_expr_ to parent exprstate
             **/
            void finish_and_continue(parserstatemachine * p_psm);

        private:
            ifexprstatetype ifxs_type_;
            /** scaffold output expression here **/
            rp<IfExprAccess> if_expr_;

        };
    } /*namespace scm*/

    namespace print {
        PPDETAIL_ATOMIC(xo::scm::ifexprstatetype);
    }
} /*namespace xo*/

/* end if_else_xs.hpp */
