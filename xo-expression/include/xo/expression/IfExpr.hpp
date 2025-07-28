/** @file IfExpr.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "Expression.hpp"
//#include <vector>
#include <string>
//#include <cstdint>

namespace xo {
    namespace scm {
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
            static rp<IfExpr> make(const rp<Expression> & test,
                                   const rp<Expression> & when_true,
                                   const rp<Expression> & when_false);

            /** downcast from Expression **/
            static bp<IfExpr> from(bp<Expression> x) {
                return bp<IfExpr>::from(x);
            }

            const rp<Expression> & test() const { return test_; }
            const rp<Expression> & when_true() const { return when_true_; }
            const rp<Expression> & when_false() const { return when_false_; }

            // ----- Expression -----

            virtual std::set<std::string> get_free_variables() const override {
                std::set<std::string> retval = test_->get_free_variables();

                std::set<std::string> free_vars;
                free_vars = when_true_->get_free_variables();
                for (const auto & s : free_vars)
                    retval.insert(s);

                free_vars = when_false_->get_free_variables();
                for (const auto & s : free_vars)
                    retval.insert(s);

                return retval;
            }

            virtual std::size_t visit_preorder(VisitFn visitor_fn) override {
                std::size_t n = 1;

                visitor_fn(this);

                n += this->test_->visit_preorder(visitor_fn);
                n += this->when_true_->visit_preorder(visitor_fn);
                n += this->when_false_->visit_preorder(visitor_fn);

                return n;
            }

            virtual std::size_t visit_layer(VisitFn visitor_fn) override {
                std::size_t n = 1;

                visitor_fn(this);

                n += this->test_->visit_layer(visitor_fn);
                n += this->when_true_->visit_layer(visitor_fn);
                n += this->when_false_->visit_layer(visitor_fn);

                return n;
            }

            virtual rp<Expression> xform_layer(TransformFn xform_fn) override {
                this->test_ = this->test_->xform_layer(xform_fn);
                this->when_true_ = this->when_true_->xform_layer(xform_fn);
                this->when_false_= this->when_false_->xform_layer(xform_fn);

                return xform_fn(this);
            }

            virtual void attach_envs(bp<Environment> p) override {
                test_->attach_envs(p);
                when_true_->attach_envs(p);
                when_false_->attach_envs(p);
            }

#ifdef NOT_USING
            virtual std::int32_t find_free_vars(std::set<bp<Variable>> * p_set) override {
                return (test_->find_free_vars(p_set)
                        + when_true_->find_free_vars(p_set)
                        + when_false_->find_free_vars(p_set));
            }
#endif

            virtual void display(std::ostream & os) const override;
            virtual std::uint32_t pretty_print(const ppindentinfo & ppi) const override;

        protected:
            /**
             *  @p ifexpr_type  type for value produced by if-expression.
             *                  same as both when_true->valuetype() and
             *                  when_false->valuetype().
             *  @p test   test-expression;  always execute
             *  @p when_true  then-branch;  executes only when test succeeds
             *  @p when_false else-branch;  executes only when test fails
             **/
            IfExpr(TypeDescr ifexpr_type,
                   rp<Expression> test,
                   rp<Expression> when_true,
                   rp<Expression> when_false)
                : Expression(exprtype::ifexpr, ifexpr_type),
                  test_{std::move(test)},
                  when_true_{std::move(when_true)},
                  when_false_{std::move(when_false)} {}

            static TypeDescr check_consistent_valuetype(const rp<Expression> & when_true,
                                                        const rp<Expression> & when_false);

            /** determine if-expr valuetype **/
            void establish_valuetype();

        protected:
            /** if:
             *    (if x y z)
             *
             *  executes x;  if true execute y; otherwise execute z
             **/
            rp<Expression> test_;
            rp<Expression> when_true_;
            rp<Expression> when_false_;
        }; /*IfExpr*/

        inline rp<IfExpr>
        make_ifexpr(const rp<Expression> & test,
                    const rp<Expression> & when_true,
                    const rp<Expression> & when_false)
        {
            return IfExpr::make(test, when_true, when_false);
        }

        class IfExprAccess : public IfExpr {
        public:
            static rp<IfExprAccess> make(rp<Expression> test,
                                         rp<Expression> when_true,
                                         rp<Expression> when_false);
            static rp<IfExprAccess> make_empty();

            void assign_test(rp<Expression> x) { test_ = std::move(x); }
            void assign_when_true(rp<Expression> x);
            void assign_when_false(rp<Expression> x);

        private:
            IfExprAccess(TypeDescr ifexpr_type,
                         rp<Expression> test,
                         rp<Expression> when_true,
                         rp<Expression> when_false)
                : IfExpr(ifexpr_type,
                         std::move(test),
                         std::move(when_true),
                         std::move(when_false)) {}
        };
    } /*namespace scm*/
} /*namespace xo*/

/** end IfExpr.hpp **/
