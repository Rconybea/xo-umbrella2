/* file DefineExpr.hpp
 *
 * author: Roland Conybeare, Jul 2024
 */

#pragma once

#include "Expression.hpp"

namespace xo {
    namespace ast {
        /** @class DefineExpr
         *  @brief Provide definition for a constant, variable or function
         *
         *  At toplevel,  introduces a new global variable.
         *  In a nested context,
         *
         *    def foo = rhsexpr
         *    body...
         *
         *  is equivalent to
         *
         *    (lambda (foo) body...)(rhsexpr)
         *
         * Promise:
         * - memory location of @ref lhs_var_ is determined when parent DefineExpr
         *   constructed, and is stable across calls to @ref DefineExpr::assign_lhs_name
         **/
        class DefineExpr : public Expression {
        public:
            static rp<DefineExpr> make(std::string name,
                                       rp<Expression> value);


            static bp<DefineExpr> from(bp<Expression> x) {
                return bp<DefineExpr>::from(x);
            }

            const std::string & lhs_name() const;
            const rp<Expression> & rhs() const { return rhs_; }

            const rp<Variable>& lhs_variable() const { return lhs_var_; }

            std::set<std::string> calc_free_variables() const;

            // ----- Expression -----

            virtual std::set<std::string> get_free_variables() const override {
                return this->free_var_set_;
            }

            virtual std::size_t visit_preorder(VisitFn visitor_fn) override {
                std::size_t n = 1;

                visitor_fn(this);

                n += rhs_->visit_preorder(visitor_fn);

                return n;
            }

            virtual std::size_t visit_layer(VisitFn visitor_fn) override {
                std::size_t n = 1;

                visitor_fn(this);

                n += this->rhs_->visit_layer(visitor_fn);

                return n;
            }

            virtual rp<Expression> xform_layer(TransformFn xform_fn) override {
                this->rhs_ = this->rhs_->xform_layer(xform_fn);

                return xform_fn(this);
            }

            virtual void attach_envs(bp<Environment> p) override {
                rhs_->attach_envs(p);
            }

            virtual void display(std::ostream & os) const override;
            virtual std::uint32_t pretty_print(const ppindentinfo & ppii) const override;

        protected:
            /**
             *
             **/
            DefineExpr(TypeDescr rhs_valuetype,
                       std::string lhs_name,
                       rp<Expression> rhs);

        protected:
            /** symbol name for this definition **/
            rp<Variable> lhs_var_;
            /** right-hand side of definition **/
            rp<Expression> rhs_;

            /** free variables for this definition **/
            std::set<std::string> free_var_set_;
        };

        /** @class DefineExprAccess
         *  @brief DefineExpr with writeable members.
         *
         *  Convenient when scaffolding a parser,
         *  e.g. see xo-parser
         **/
        class DefineExprAccess : public DefineExpr {
        public:
            static rp<DefineExprAccess> make(std::string lhs_name,
                                             rp<Expression> rhs);
            static rp<DefineExprAccess> make_empty();

            void assign_lhs_name(const std::string & x);
            void assign_rhs(const rp<Expression> & x);

        private:
            DefineExprAccess(TypeDescr rhs_valuetype,
                             std::string lhs_name,
                             rp<Expression> rhs)
                : DefineExpr(rhs_valuetype,
                             std::move(lhs_name),
                             std::move(rhs))
                {}
        };

    } /*namespace ast*/
} /*namespace xo*/

/* end DefineExpr.hpp */
