/** @file IfExpr.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "Expression.hpp"
#include <vector>
#include <string>
//#include <cstdint>

namespace xo {
    namespace ast {
        /** @class IfExpr
         *  @brief abstract syntax tree for a function definition
         *
         **/
        class IfExpr : public Expression {
        public:
            /** @p test   test-expression;  always execute
             *  @p when_true  then-branch;  executes only when test succeeds
             *  @p when_false else-branch;  executes only when test fails
             **/
            IfExpr(const ref::rp<Expression> & test,
                   const ref::rp<Expression> & when_true,
                   const ref::rp<Expression> & when_false)
                : Expression(exprtype::ifexpr),
                  test_{test},
                  when_true_{when_true},
                  when_false_{when_false} {}

            /** downcast from Expression **/
            static ref::brw<IfExpr> from(ref::brw<Expression> x) {
                return ref::brw<IfExpr>::from(x);
            }

            const ref::rp<Expression> & test() const { return test_; }
            const ref::rp<Expression> & when_true() const { return when_true_; }
            const ref::rp<Expression> & when_false() const { return when_false_; }

            // ----- Expression -----

            virtual void display(std::ostream & os) const override;

        private:
            /** if:
             *    (if x y z)
             *
             *  executes x;  if true execute y; otherwise execute z
             **/
            ref::rp<Expression> test_;
            ref::rp<Expression> when_true_;
            ref::rp<Expression> when_false_;
        }; /*IfExpr*/

        inline ref::rp<IfExpr>
        make_ifexpr(const ref::rp<Expression> & test,
                    const ref::rp<Expression> & when_true,
                    const ref::rp<Expression> & when_false)
        {
            return new IfExpr(test, when_true, when_false);
        }
    } /*namespace ast*/
} /*namespace xo*/

/** end IfExpr.hpp **/
