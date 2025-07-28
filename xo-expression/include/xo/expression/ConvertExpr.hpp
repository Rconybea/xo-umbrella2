/* file ConvertExpr.hpp
 *
 * author: Roland Conybeare, Aug 2024
 */

#pragma once

#include "Expression.hpp"

namespace xo {
    namespace scm {
        /** @class Convertexpr
         *  @brief Convenience for automatic type conversion
         *
         *  This is equivalent to calling a built-in primitive
         *  that performs the conversion.
         *
         *  We rely on this for convenience,  for example to parse
         *  code like
         *
         *    def foo : i16 = 0
         **/
        class ConvertExpr : public Expression {
        public:
            static rp<ConvertExpr> make(TypeDescr dest_type,
                                        rp<Expression> arg);

            static bp<ConvertExpr> from(bp<Expression> x) {
                return bp<ConvertExpr>::from(x);
            }

            const rp<Expression> & arg() const { return arg_; }

            // ----- Expression -----

            virtual std::set<std::string> get_free_variables() const override;

            virtual std::size_t visit_preorder(VisitFn visitor_fn) override {
                std::size_t n = 1;

                visitor_fn(this);

                n += arg_->visit_preorder(visitor_fn);

                return n;
            }

            virtual std::size_t visit_layer(VisitFn visitor_fn) override {
                std::size_t n = 1;

                visitor_fn(this);

                n += this->arg_->visit_layer(visitor_fn);

                return n;
            }

            virtual rp<Expression> xform_layer(TransformFn xform_fn) override {
                this->arg_ = this->arg_->xform_layer(xform_fn);

                return xform_fn(this);
            }

            virtual void attach_envs(bp<Environment> p) override {
                arg_->attach_envs(p);
            }

            virtual void display(std::ostream & os) const override;
            virtual std::uint32_t pretty_print(const ppindentinfo & ppii) const override;

        protected:
            ConvertExpr(TypeDescr dest_type,
                        rp<Expression> arg)
                : Expression(exprtype::convert, dest_type),
                  arg_{std::move(arg)}
                {}

        protected:
            /** source expression.  Convert
             *  @c arg_->valuetype() to @c dest_type_
             **/
            rp<Expression> arg_;
        };

        /** @class ConvertExprAccess
         *  @brief ConvertExpr with writeable members.
         *
         *  Convenient when scaffolding a parser,
         *  e.g. see xo-parser
         **/
        class ConvertExprAccess : public ConvertExpr {
        public:
            static rp<ConvertExprAccess> make(TypeDescr dest_type,
                                              rp<Expression> arg);
            static rp<ConvertExprAccess> make_empty();

            void assign_arg(rp<Expression> arg) { this->arg_ = std::move(arg); }

        private:
            ConvertExprAccess(TypeDescr dest_type,
                              rp<Expression> arg)
                : ConvertExpr(dest_type,
                              std::move(arg))
                {}
        };
    } /*namespace scm*/
} /*namespace xo*/


/* end ConvertExpr.hpp */
