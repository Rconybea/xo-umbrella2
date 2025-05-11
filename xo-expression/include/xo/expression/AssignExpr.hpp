/* file AssignExpr.hpp
 *
 * author: Roland Conybeare, Aug 2024
 */

#pragma once

#include "Expression.hpp"
#include "Variable.hpp"

namespace xo {
    namespace ast {
        /** @class AssignExpr
         *  @brief Provide expression for assigning to a variable
         *
         *    def pi = 3.14159265;
         *    def foo = 0.0;
         *    foo := pi / 2;
         **/
        class AssignExpr : public Expression {
        public:
            static rp<AssignExpr> make(const rp<Variable> & lhs,
                                       const rp<Expression> & rhs);

            static ref::brw<AssignExpr> from(ref::brw<Expression> x) {
                return ref::brw<AssignExpr>::from(x);
            }

            const rp<Variable> & lhs() const { return lhs_; }
            const rp<Expression> & rhs() const { return rhs_; }

            std::set<std::string> calc_free_variables() const;

            // ----- inherited from Expression -----

            virtual std::set<std::string> get_free_variables() const override;
            virtual std::size_t visit_preorder(VisitFn visitor_fn) override;
            virtual std::size_t visit_layer(VisitFn visitor_fn) override;
            virtual rp<Expression> xform_layer(TransformFn xform_fn) override;
            virtual void attach_envs(ref::brw<Environment> p) override;

            virtual void display(std::ostream & os) const override;

        private:
            AssignExpr(const rp<Variable> & lhs,
                       const rp<Expression> & rhs);

        private:
            /** assign to this variable. **/
            rp<Variable> lhs_;
            /** assign value of this expression to variable @p lhs **/
            rp<Expression> rhs_;

            /** free variables for this assignment **/
            std::set<std::string> free_var_set_;
        };
    } /*namespace ast*/
} /*namespace xo*/


/* end AssignExpr.hpp */
