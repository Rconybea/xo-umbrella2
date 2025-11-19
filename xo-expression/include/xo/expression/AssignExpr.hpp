/* file AssignExpr.hpp
 *
 * author: Roland Conybeare, Aug 2024
 */

#pragma once

#include "Expression.hpp"
#include "Variable.hpp"

namespace xo {
    namespace scm {
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

            static bp<AssignExpr> from(bp<Expression> x) {
                return bp<AssignExpr>::from(x);
            }

            const rp<Variable> & lhs() const { return lhs_; }
            const rp<Expression> & rhs() const { return rhs_; }

            std::set<std::string> calc_free_variables() const;

            // ----- inherited from Expression -----

            virtual std::set<std::string> get_free_variables() const override;
            virtual std::size_t visit_preorder(VisitFn visitor_fn) override;
            virtual std::size_t visit_layer(VisitFn visitor_fn) override;
            virtual rp<Expression> xform_layer(TransformFn xform_fn) override;
            virtual void attach_envs(bp<SymbolTable> p) override;

            virtual void display(std::ostream & os) const override;
            virtual std::uint32_t pretty_print(const ppindentinfo & ppii) const override;

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
    } /*namespace scm*/
} /*namespace xo*/


/* end AssignExpr.hpp */
