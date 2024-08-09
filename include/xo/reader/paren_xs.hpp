/** @file paren_xs.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "exprstate.hpp"
//#include <cstdint>

namespace xo {
    namespace scm {
        enum class parenexprstatetype {
            invalid = -1,

            lparen_0,
            lparen_1,

            n_parenexprstatetype,
        };

        /** @class paren_xs
         *  @brief state machine for handling parentheses in expressions
         **/
        class paren_xs : public exprstate {
        public:
            paren_xs();
            virtual ~paren_xs() = default;

            static const paren_xs * from(const exprstate * x) { return dynamic_cast<const paren_xs *>(x); }

            static std::unique_ptr<paren_xs> lparen_0();

            virtual bool admits_definition() const override;
            virtual bool admits_symbol() const override;
            virtual bool admits_colon() const override;
            virtual bool admits_semicolon() const override;
            virtual bool admits_singleassign() const override;
            virtual bool admits_leftparen() const override;
            virtual bool admits_rightparen() const override;
            virtual bool admits_f64() const override;

            // virtual void on_f64(..) override
            virtual void on_def(exprstatestack * p_stack) override;

            virtual void on_expr(ref::brw<Expression> expr,
                                 exprstatestack * p_stack,
                                 rp<Expression> * p_emit_expr) override;
            virtual void on_symbol(const std::string & symbol,
                                   exprstatestack * p_stack,
                                   rp<Expression> * p_emit_expr) override;
            virtual void on_typedescr(TypeDescr td,
                                      exprstatestack * /*p_stack*/,
                                      rp<Expression> * /*p_emit_expr*/) override;

            virtual void on_symbol(const token_type & tk,
                                   exprstatestack * p_stack,
                                   rp<Expression> * p_emit_expr) override;
            virtual void on_colon(exprstatestack * p_stack) override;
            virtual void on_semicolon(exprstatestack * p_stack,
                                      rp<Expression> * /*p_emit_expr*/) override;
            virtual void on_singleassign(exprstatestack * p_stack) override;
            virtual void on_leftparen(exprstatestack * p_stack,
                                      rp<Expression> * /*p_emit_expr*/) override;
            virtual void on_rightparen(exprstatestack * p_stack,
                                       rp<Expression> * /*p_emit_expr*/) override;
            virtual void on_f64(const token_type & tk,
                                exprstatestack * p_stack,
                                rp<Expression> * /*p_emit_expr*/) override;

            virtual void print(std::ostream & os) const override;

        private:
            /**
             *  ( foo ... )
             *   ^
             *   |
             *   lparen_0
             **/
            parenexprstatetype parenxs_type_;
        };
    } /*namespace scm*/
} /*namespace xo*/


/** end paren_xs.hpp **/
