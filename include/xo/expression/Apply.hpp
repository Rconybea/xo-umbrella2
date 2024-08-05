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

            /** downcast from Expression **/
            static ref::brw<Apply> from(ref::brw<Expression> x) {
                return ref::brw<Apply>::from(x);
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

            virtual void attach_envs(ref::brw<Environment> p) override {
                fn_->attach_envs(p);

                for (const auto & arg : argv_)
                    arg->attach_envs(p);
            }

            virtual void display(std::ostream & os) const override;

        private:
            Apply(TypeDescr apply_valuetype,
                  const rp<Expression> & fn,
                  const std::vector<rp<Expression>> & argv)
                : Expression(exprtype::apply, apply_valuetype),
                  fn_{fn}, argv_(argv)
                {}

        private:
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

    } /*namespace ast*/
} /*namespace xo*/


/** end Apply.hpp **/
