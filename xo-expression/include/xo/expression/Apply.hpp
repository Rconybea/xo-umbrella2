/** @file Apply.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "Expression.hpp"

//#include <cstdint>

namespace xo {
    namespace ast {

        /** @class Apply
         *  @brief syntax for a function call.
         *
         *  In general we don't know function to be invoked
         *  until runtime,  depending on the nature of Expression.
         **/
        class Apply : public Expression {
        public:
            using TypeDescr = xo::reflect::TypeDescr;

        public:
            /** create new apply-expression instance
             **/
            static rp<Apply> make(const rp<Expression> & fn,
                                  const std::vector<rp<Expression>> & argv);

            /** create apply-expression to compare two 64-bit integers **/
            static rp<Apply> make_cmp_eq_i64(const rp<Expression> & lhs,
                                             const rp<Expression> & rhs);
            /** create apply-expression to compare two 64-bit integers **/
            static rp<Apply> make_cmp_ne_i64(const rp<Expression> & lhs,
                                             const rp<Expression> & rhs);
            /** create apply-expression for less-than comparison of two 64-bit integers **/
            static rp<Apply> make_cmp_lt_i64(const rp<Expression> & lhs,
                                             const rp<Expression> & rhs);
            /** create apply-expression for less-than-or-equal comparison of two 64-bit integers **/
            static rp<Apply> make_cmp_le_i64(const rp<Expression> & lhs,
                                             const rp<Expression> & rhs);
            /** create apply-expression for greater-than comparison of two 64-bit integers **/
            static rp<Apply> make_cmp_gt_i64(const rp<Expression> & lhs,
                                             const rp<Expression> & rhs);
            /** create apply-expression for greater-than-or-equal comparison of two 64-bit integers **/
            static rp<Apply> make_cmp_ge_i64(const rp<Expression> & lhs,
                                             const rp<Expression> & rhs);
            /** create apply-expression to add two 64-bit integers **/
            static rp<Apply> make_add2_i64(const rp<Expression> & lhs,
                                           const rp<Expression> & rhs);
            /** create apply-expression to subtract two 64-bit integers **/
            static rp<Apply> make_sub2_i64(const rp<Expression> & lhs,
                                           const rp<Expression> & rhs);
            /** create apply-expression to multiply two 64-bit integers **/
            static rp<Apply> make_mul2_i64(const rp<Expression> & lhs,
                                           const rp<Expression> & rhs);
            /** create apply-expression to divide two 64-bit integers **/
            static rp<Apply> make_div2_i64(const rp<Expression> & lhs,
                                           const rp<Expression> & rhs);

            /** create apply-expression to add two 64-bit floating-point numbers **/
            static rp<Apply> make_add2_f64(const rp<Expression> & lhs,
                                           const rp<Expression> & rhs);
            /** create apply-expression to subtract two 64-bit floating-point numbers **/
            static rp<Apply> make_sub2_f64(const rp<Expression> & lhs,
                                           const rp<Expression> & rhs);
            /** create apply-expression to multiply two 64-bit floating-point numbers **/
            static rp<Apply> make_mul2_f64(const rp<Expression> & lhs,
                                           const rp<Expression> & rhs);
            /** create apply-expression to divide two 64-bit floating-point numbers **/
            static rp<Apply> make_div2_f64(const rp<Expression> & lhs,
                                           const rp<Expression> & rhs);

            /** downcast from Expression **/
            static bp<Apply> from(bp<Expression> x) {
                return bp<Apply>::from(x);
            }

            const rp<Expression> & fn() const { return fn_; }
            const std::vector<rp<Expression>> & argv() const { return argv_; }

            virtual std::set<std::string> get_free_variables() const override {
                std::set<std::string> retval = fn_->get_free_variables();

                for (const auto & arg : argv_) {
                    std::set<std::string> arg_free_set
                        = arg->get_free_variables();

                    for (const auto & name : arg_free_set)
                        retval.insert(name);
                }

                return retval;
            }

            virtual std::size_t visit_preorder(VisitFn visitor_fn) override {
                std::size_t n = 1;

                visitor_fn(this);

                n += fn_->visit_preorder(visitor_fn);

                for (const auto & arg : argv_)
                    n += arg->visit_preorder(visitor_fn);

                return n;
            }

            virtual std::size_t visit_layer(VisitFn visitor_fn) override {
                std::size_t n = 1;

                visitor_fn(this);

                n += fn_->visit_layer(visitor_fn);

                for (const auto & arg : argv_)
                    n += arg->visit_layer(visitor_fn);

                return n;
            }

            virtual rp<Expression> xform_layer(TransformFn xform_fn) override {
                this->fn_ = fn_->xform_layer(xform_fn);

                for (auto & arg : argv_)
                    arg = arg->xform_layer(xform_fn);

                return xform_fn(this);
            }

            virtual void attach_envs(bp<Environment> p) override;

            virtual void display(std::ostream & os) const override;
            virtual std::uint32_t pretty_print(const ppindentinfo & ppii) const override;

        protected:
            Apply(TypeDescr apply_valuetype,
                  const rp<Expression> & fn,
                  const std::vector<rp<Expression>> & argv)
                : Expression(exprtype::apply, apply_valuetype),
                  fn_{fn}, argv_(argv)
                {}

        protected:
            /** function to invoke **/
            rp<Expression> fn_;
            /** argument expressions,  in l-to-r order **/
            std::vector<rp<Expression>> argv_;
        }; /*Apply*/

#ifdef NOT_USING
        namespace detail {
            /** Use:
             **   std::vector<ref::rp<Expression>>
             **/

            template <typename... Args>
            struct apply_push_args;

            template <>
            struct apply_push_args<> {
                static void push_all(std::vector<ref::rp<Expression>> * /*p_argv*/) {}
            };

            template <typename Arg1, typename... Rest>
            struct apply_push_args<Arg1, Rest...> {
                static void push_all(std::vector<ref::rp<Expression>> * p_argv,
                                     const ref::rp<Expression> & x, Rest... rest)
                    {
                        p_argv->push_back(x);
                        apply_push_args<Rest...>::push_all(p_argv, rest...);
                    };
            };
        }
#endif

        /* reminder: initializer-lists are compile-time only */
        inline rp<Apply>
        make_apply(const rp<Expression> & fn,
                   const std::initializer_list<rp<Expression>> args) {
            std::vector<rp<Expression>> argv(args);
            return Apply::make(fn, argv);
        } /*make_apply*/

        /** @class ApplyAccess
         *  @brief Apply with writeable members
         *
         *  Convenient when scaffolding a parser,
         *  e.g. see xo-parser
         **/
        class ApplyAccess : public Apply {
        public:
            static rp<ApplyAccess> make_empty();

            /** assign function being called to @p fn **/
            void assign_fn(const rp<Expression>& fn);
            /** assign expression for argument i, counting from 1.
             *  can use @p i = 0 as alternative to @ ref assign_fn
             **/
            void assign_arg(size_t i, const rp<Expression>& arg);

            // inherited from GeneralizedExpression..
            // void assign_valuetype(TypeDescr apply_valuetype);

        private:
            ApplyAccess(TypeDescr apply_valuetype,
                        const rp<Expression>& fn,
                        const std::vector<rp<Expression>>& argv)
                : Apply(apply_valuetype, fn, argv) {}
        };

    } /*namespace ast*/
} /*namespace xo*/


/** end Apply.hpp **/
