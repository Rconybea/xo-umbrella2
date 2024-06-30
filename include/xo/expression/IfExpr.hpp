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
            using TypeDescr = xo::reflect::TypeDescr;

        public:
            /** create expression for conditional execution of
             *  @p when_true or @p when_false, depending on result
             *  of evaluating expression @p test
             **/
            static ref::rp<IfExpr> make(const ref::rp<Expression> & test,
                                        const ref::rp<Expression> & when_true,
                                        const ref::rp<Expression> & when_false);

            /** downcast from Expression **/
            static ref::brw<IfExpr> from(ref::brw<Expression> x) {
                return ref::brw<IfExpr>::from(x);
            }

            const ref::rp<Expression> & test() const { return test_; }
            const ref::rp<Expression> & when_true() const { return when_true_; }
            const ref::rp<Expression> & when_false() const { return when_false_; }

            // ----- Expression -----

            virtual std::size_t visit_preorder(VisitFn visitor_fn) override {
                std::size_t n = 1;

                visitor_fn(this);

                n += this->when_true_->visit_preorder(visitor_fn);
                n += this->when_false_->visit_preorder(visitor_fn);

                return n;
            }

            virtual void attach_envs(ref::brw<Environment> p) override {
                test_->attach_envs(p);
                when_true_->attach_envs(p);
                when_false_->attach_envs(p);
            }

#ifdef NOT_USING
            virtual std::int32_t find_free_vars(std::set<ref::brw<Variable>> * p_set) override {
                return (test_->find_free_vars(p_set)
                        + when_true_->find_free_vars(p_set)
                        + when_false_->find_free_vars(p_set));
            }
#endif

            virtual void display(std::ostream & os) const override;

        private:
            /**
             *  @p ifexpr_type  type for value produced by if-expression.
             *                  same as both when_true->valuetype() and
             *                  when_false->valuetype().
             *  @p test   test-expression;  always execute
             *  @p when_true  then-branch;  executes only when test succeeds
             *  @p when_false else-branch;  executes only when test fails
             **/
            IfExpr(TypeDescr ifexpr_type,
                   const ref::rp<Expression> & test,
                   const ref::rp<Expression> & when_true,
                   const ref::rp<Expression> & when_false)
                : Expression(exprtype::ifexpr, ifexpr_type),
                  test_{test},
                  when_true_{when_true},
                  when_false_{when_false} {}

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
            return IfExpr::make(test, when_true, when_false);
        }
    } /*namespace ast*/
} /*namespace xo*/

/** end IfExpr.hpp **/
