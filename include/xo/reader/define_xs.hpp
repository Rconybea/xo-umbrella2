/** @file define_xs.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "xo/expression/DefineExpr.hpp"
#include "xo/expression/ConvertExpr.hpp"
#include "exprstate.hpp"
//#include <cstdint>

namespace xo {
    namespace scm {
        enum class defexprstatetype {
            invalid = -1,

            def_0,
            def_1,
            def_2,
            def_3,
            def_4,
            def_5,

            n_defexprstatetype,
        };

        /** @class define_xs
         *  @brief state to provide parsing of a define-expression
         **/
        class define_xs : public exprstate {
        public:
            using DefineExprAccess = xo::ast::DefineExprAccess;
            using ConvertExprAccess = xo::ast::ConvertExprAccess;

        public:
            define_xs(rp<DefineExprAccess> def_expr);
            virtual ~define_xs() = default;

            static const define_xs * from(const exprstate * x) { return dynamic_cast<const define_xs *>(x); }
            static std::unique_ptr<define_xs> def_0();

            defexprstatetype defxs_type() const { return defxs_type_; }

            virtual bool admits_definition() const override;
            virtual bool admits_symbol() const override;
            virtual bool admits_colon() const override;
            virtual bool admits_semicolon() const override;
            virtual bool admits_singleassign() const override;
            virtual bool admits_leftparen() const override;
            virtual bool admits_rightparen() const override;

            // virtual void on_f64(..) override
            virtual void on_expr(ref::brw<Expression> expr,
                                 exprstatestack * p_stack,
                                 rp<Expression> * p_emit_expr) override;
            virtual void on_symbol(const std::string & symbol_name,
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
            virtual void on_leftparen_token(const token_type & tk,
                                            exprstatestack * p_stack,
                                            rp<Expression> * /*p_emit_expr*/) override;
            virtual void on_rightparen_token(const token_type & tk,
                                             exprstatestack * p_stack,
                                             rp<Expression> * /*p_emit_expr*/) override;
            virtual void on_f64_token(const token_type & tk,
                                      exprstatestack * p_stack,
                                      rp<Expression> * /*p_emit_expr*/) override;

            virtual void print(std::ostream & os) const override;

        private:
            /**
             *   def foo : f64 = 1 ;
             *  ^   ^   ^ ^   ^ ^ ^ ^
             *  |   |   | |   | | | (done)
             *  |   |   | |   | | def_4:expect_rhs_expression:expr_progress
             *  |   |   | |   | def_4:expect_rhs_expression
             *  |   |   | |   def_3
             *  |   |   | def_2:expect_type
             *  |   |   def_1
             *  |   def_0:expect_symbol
             *  expect_toplevel_expression_sequence
             *
             *   def_0:expect_symbol: got 'def' keyword, symbol to follow
             *   def_1: got symbol name
             *   def_2:expect_symbol got (optional) colon, type name to follow
             *   def_3: got symbol type
             *   def_4:expect_rhs_expression got (optional) equal sign, value to follow
             *   (done): definition complete,  pop exprstate from stack
             *
             **/
            defexprstatetype defxs_type_;
            /** scaffold a define-expression here **/
            rp<DefineExprAccess> def_expr_;
            /** scafford a convert-expression here.
             *  May be nested within a def_expr
             **/
            rp<ConvertExprAccess> cvt_expr_;
        };
    } /*namespace scm*/
} /*namespace xo*/


/** end define_xs.hpp **/
