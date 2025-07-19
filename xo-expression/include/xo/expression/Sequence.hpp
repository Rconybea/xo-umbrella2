/** @file Sequence.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "Expression.hpp"

namespace xo {
    namespace ast {
        class Sequence : public Expression {
        public:
            Sequence(const std::vector<rp<Expression>> & xv)
                : Expression(exprtype::sequence,
                             xv[xv.size() - 1]->valuetype()),
                  expr_v_(xv) {}

            static rp<Sequence> make(const std::vector<rp<Expression>> & xv) { return new Sequence(xv); }

            std::size_t size() const { return expr_v_.size(); }
            const rp<Expression> & operator[](std::size_t i) const { return expr_v_[i]; }

            // ----- from Expression -----

            /** note: broken if .expr_v_ contains any def-exprs
             *  (will treat references to so-defined vars as free).
             *  must rewrite these first
             **/
            virtual std::set<std::string> get_free_variables() const override;
            virtual std::size_t visit_preorder(VisitFn visitor_fn) override;
            /** note: borken if .expr_v_ contains any def-exprs **/
            virtual std::size_t visit_layer(VisitFn visitor_fn) override;
            virtual rp<Expression> xform_layer(TransformFn visitor_fn) override;
            virtual void attach_envs(bp<Environment> parent) override;

            // ----- from GeneralizedExpression ----

            virtual void display(std::ostream & os) const override;
            virtual std::uint32_t pretty_print(const ppindentinfo & ppii) const override;

        private:
            /** sequence of expressions;  evaluate in left-to-right order.
             **/
            std::vector<rp<Expression>> expr_v_;
        };
    } /*namespace ast*/

} /*namespace xo*/

/** end Sequence.hpp **/
