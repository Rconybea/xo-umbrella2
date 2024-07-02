/** @file Lambda.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "Expression.hpp"
#include "FunctionInterface.hpp"
#include "Variable.hpp"
#include "LocalEnv.hpp"
#include <vector>
#include <string>
//#include <cstdint>

namespace xo {
    namespace ast {
        /** @class Lambda
         *  @brief abstract syntax tree for a function definition
         *
         **/
        class Lambda : public FunctionInterface {
        public:
            /**
             *  @p name   Name for this lambda -- must be unique
             *  @p argv   Formal parameters, in left-to-right order
             *  @p body   Expression for body of this function
             **/
            static ref::rp<Lambda> make(const std::string & name,
                                        const std::vector<ref::rp<Variable>> & argv,
                                        const ref::rp<Expression> & body);

            /** downcast from Expression **/
            static ref::brw<Lambda> from(ref::brw<Expression> x) {
                return ref::brw<Lambda>::from(x);
            }

            const std::string & type_str() const { return type_str_; }
            const std::vector<ref::rp<Variable>> & argv() const { return local_env_->argv(); }
            const ref::rp<Expression> & body() const { return body_; }

            bool needs_closure_flag() const { return !free_var_set_.empty(); }

            // ----- FunctionInterface -----

            virtual const std::string & name() const override { return name_; }
            /** return number of arguments expected by this function **/
            virtual int n_arg() const override { return local_env_->n_arg(); }
            virtual TypeDescr fn_retval() const override { return body_->valuetype(); }
            virtual TypeDescr fn_arg(uint32_t i) const override { return local_env_->fn_arg(i); }

            // ----- Expression -----

            virtual std::set<std::string> get_free_variables() const override {
                return this->free_var_set_;
            }

            virtual std::size_t visit_preorder(VisitFn visitor_fn) override {
                std::size_t n = 1;

                visitor_fn(this);

                for (const auto & arg : local_env_->argv())
                    n += arg->visit_preorder(visitor_fn);

                n += body_->visit_preorder(visitor_fn);

                return n;
            }

            virtual void attach_envs(ref::brw<Environment> p) override {
                local_env_->assign_parent(p);
            }

            virtual void display(std::ostream & os) const override;

        private:
            /** @param lambda_type.  function type for this lambda.
             *  We arbitrarily choose the form "Retval(*)(Args...)"
             **/
            Lambda(const std::string & name,
                   TypeDescr lambda_type,
                   const ref::rp<LocalEnv> & local_env,
                   const ref::rp<Expression> & body);

            /** compute free-variable set for this lambda **/
            std::set<std::string> calc_free_variables() const;

        private:
            /** lambda name.  Initially supporting only form like
             *    (define (foo x y z)
             *      (+ (* x x) (* y y) (* z z)))
             *
             *  In any case need to supply names for distinct things-for-which-code-is-generated
             *  so that they can be linked etc.
             **/
            std::string name_;
            /** e.g.
             *    "double(double,double)"  for function of two doubles that returns a double
             **/
            std::string type_str_;
            /** function body **/
            ref::rp<Expression> body_;

            /** free variables for this lambda **/
            std::set<std::string> free_var_set_;

            /** established (once) by @ref attach_envs.
             *
             * @note data dependency on ancestor expressions that don't exist yet
             * when Lambda constructor runs,  so we need to assign @ref local_env_
             * later.
             **/
            ref::rp<LocalEnv> local_env_;
        }; /*Lambda*/

        inline ref::rp<Lambda>
        make_lambda(const std::string & name,
                    const std::vector<ref::rp<Variable>> & argv,
                    const ref::rp<Expression> & body)
        {
            return Lambda::make(name, argv, body);
        }
    } /*namespace ast*/
} /*namespace xo*/

/** end Lambda.hpp **/
