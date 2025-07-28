/** @file Lambda.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "Expression.hpp"
#include "FunctionInterface.hpp"
#include "Variable.hpp"
#include "LocalEnv.hpp"
#include <map>
#include <vector>
#include <string>
//#include <cstdint>

namespace xo {
    namespace scm {
        /** @class Lambda
         *  @brief abstract syntax tree for a function definition
         *
         **/
        class Lambda : public FunctionInterface {
        public:
            /**
             *  @p name.  Name for this lambda -- must be unique
             *  @p lambda_type.  Function signature
             *  @p local_env.  Environment with formals as content
             *  @p body.  Expression for lambda function body
             **/
            static rp<Lambda> make(const std::string & name,
                                   TypeDescr lambda_type,
                                   const rp<LocalEnv> & local_env,
                                   const rp<Expression> & body);

            /**
             *  @p name   Name for this lambda -- must be unique
             *  @p argv   Formal parameters, in left-to-right order
             *  @p body   Expression for body of this function
             *  @p parent_env  Environment for enclosing lexical scope
             **/
            static rp<Lambda> make(const std::string & name,
                                   const std::vector<rp<Variable>> & argv,
                                   const rp<Expression> & body,
                                   const rp<Environment> & parent_env);

            /**
             *  @p name   Name for this lambda -- must be unique
             *  @p env    Environment with {name,type} for each formal parameter
             *  @p body   Expression for body of function
             **/
            static rp<Lambda> make_from_env(const std::string & name,
                                            const rp<LocalEnv> & env,
                                            TypeDescr explicit_return_td,
                                            const rp<Expression> & body);

            /** create type description for lambda with arguments @p argv
             *  and return type @p return_td
             **/
            static TypeDescr assemble_lambda_td(const std::vector<rp<Variable>> & argv,
                                                TypeDescr return_td);

            /** create type description for lambda with arguments @p argv
             *  and body expression @p body.
             *  @p explicit_return_td will be used if non-null.
             *  otherwise use @p body valuetype
             **/
            static TypeDescr assemble_lambda_td(const std::vector<rp<Variable>> & argv,
                                                TypeDescr explicit_return_td,
                                                const rp<Expression> & body);

            /** downcast from Expression **/
            static bp<Lambda> from(bp<Expression> x) {
                return bp<Lambda>::from(x);
            }

            const std::string & type_str() const { return type_str_; }
            const std::vector<rp<Variable>> & argv() const { return local_env_->argv(); }
            const rp<Expression> & body() const { return body_; }

            const std::string& i_argname(int i_arg) const { return local_env_->lookup_arg(i_arg)->name(); }
            bool needs_closure_flag() const { return !free_var_set_.empty(); }
            bool is_captured(const std::string& var) const { return (captured_var_set_.find(var) != captured_var_set_.end()); }

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

            virtual std::size_t visit_layer(VisitFn visitor_fn) override {
                std::size_t n = 1;

                visitor_fn(this);

                return n;
            }

            virtual rp<Expression> xform_layer(TransformFn /*xform_fn*/) override {
                /* a layer is bounded by lambdas,  don't enter them */
                return this;
            }

            virtual void attach_envs(bp<Environment> p) override;

            virtual void display(std::ostream & os) const override;
            virtual std::uint32_t pretty_print(const ppindentinfo & ppii) const override;

        protected:
            /** create string description for function signature,
             *  consistent with c++ expectation
             **/
            static std::string assemble_type_str(TypeDescr lambda_td);

            /** @param lambda_type.  function type for this lambda.
             *  We arbitrarily choose the form "Retval(*)(Args...)"
             **/
            Lambda(const std::string & name,
                   TypeDescr lambda_type,
                   const rp<LocalEnv> & local_env,
                   const rp<Expression> & body);

            /** compute free-variable set for this lambda **/
            std::set<std::string> calc_free_variables() const;

            /** ensure at most one Variable instance with a particular name
             *  in this lambda,  but ignore nested lambdas.
             *
             *  Goal is to unify variables that can use the same binding
             *  path to determine memory location at runtime.
             **/
            std::map<std::string, rp<Variable>> regularize_layer_vars();

            /** compute derived members
             *  (type_str_, free_var_set_, captured_var_set_, layer_var_map_,
             *   nested_lambda_map_)
             *  once .body_ is established
             **/
            void complete_assembly_from_body();

        protected:
            /** lambda name.  Initially supporting only form like
             *    (define (foo x y z)
             *      (+ (* x x) (* y y) (* z z)))
             *
             *  In any case need to supply names for distinct
             *  things-for-which-code-is-generated so that they can be linked etc.
             **/
            std::string name_;
            /** e.g.
             *    "double(double,double)"  for function of two doubles that
             *  returns a double
             **/
            std::string type_str_;
            /** function body **/
            rp<Expression> body_;

            /** free variables for this lambda **/
            std::set<std::string> free_var_set_;

            /** variables that appear free in some nested lambda **/
            std::set<std::string> captured_var_set_;

            /** map giving unique identity to each variable appearing in this layer.
             *  includes:
             *  - formal parameters
             *  - free variables in @ref body_
             *  excludes:
             *  - any variables appearing in nested lambdas
             *    (whether formals or free variables)
             **/
            std::map<std::string, rp<Variable>> layer_var_map_;

            /** all lambdas nested once inside this lambda's body **/
            std::map<std::string, bp<Lambda>> nested_lambda_map_;

            /** established (once) by @ref attach_envs.
             *
             * @note data dependency on ancestor expressions that don't exist yet
             * when Lambda constructor runs,  so we need to assign @ref local_env_
             * later.
             **/
            rp<LocalEnv> local_env_;
        }; /*Lambda*/

        inline rp<Lambda>
        make_lambda(const std::string & name,
                    const std::vector<rp<Variable>> & argv,
                    const rp<Expression> & body,
                    const rp<Environment> & parent_env)
        {
            return Lambda::make(name, argv, body, parent_env);
        }

        class LambdaAccess : public Lambda {
        public:
            static rp<LambdaAccess> make(const std::string & name,
                                         const std::vector<rp<Variable>> & argv,
                                         const rp<Expression> & body,
                                         const rp<Environment> & parent_env);
            static rp<LambdaAccess> make_empty();

            /** assign body + compute derived members
             *  (see complete_assembly_from_body())
             **/
            void assign_body(const rp<Expression> & body);

        private:
            /** lambda_type, body can be null here,
             *  in which case fill in with assign methods
             **/
            LambdaAccess(const std::string & name,
                         TypeDescr lambda_type,
                         const rp<LocalEnv> & local_env,
                         const rp<Expression> & body);


        };
    } /*namespace scm*/
} /*namespace xo*/

/** end Lambda.hpp **/
