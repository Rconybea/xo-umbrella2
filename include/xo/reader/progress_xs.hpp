/** @file progress_xs.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "exprstate.hpp"
//#include <cstdint>

namespace xo {
    namespace scm {
        /** @class progress_xs
         *  @brief state machine for parsing a schematica runtime-value-expression
         **/
        class progress_xs : public exprstate {
        public:
            progress_xs(rp<Expression> valex);
            virtual ~progress_xs() = default;

            static const progress_xs * from(const exprstate * x) { return dynamic_cast<const progress_xs *>(x); }

            static std::unique_ptr<progress_xs> make(rp<Expression> valex);

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
            virtual void on_symbol_token(const token_type & tk,
                                         exprstatestack * p_stack,
                                         rp<Expression> * p_emit_expr) override;
            virtual void on_typedescr(TypeDescr td,
                                      exprstatestack * /*p_stack*/,
                                      rp<Expression> * /*p_emit_expr*/) override;
            virtual void on_colon_token(const token_type & tk,
                                        exprstatestack * p_stack) override;
            virtual void on_semicolon_token(const token_type & tk,
                                            exprstatestack * p_stack,
                                            rp<Expression> * /*p_emit_expr*/) override;
            virtual void on_singleassign_token(const token_type & tk,
                                               exprstatestack * p_stack) override;
            virtual void on_leftparen(exprstatestack * p_stack,
                                      rp<Expression> * /*p_emit_expr*/) override;
            virtual void on_rightparen(exprstatestack * p_stack,
                                       rp<Expression> * /*p_emit_expr*/) override;
            virtual void on_f64(const token_type & tk,
                                exprstatestack * p_stack,
                                rp<Expression> * /*p_emit_expr*/) override;

            virtual void print(std::ostream & os) const override;

        private:
            /** populate an expression here **/
            rp<Expression> gen_expr_;
        };
    } /*namespace scm*/
} /*namespace xo*/


/** end progress_xs.hpp **/
