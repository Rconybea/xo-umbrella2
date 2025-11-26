/* @file Lambda.cpp */

#include "Lambda.hpp"
#include "exprtype.hpp"
#include "pretty_expression.hpp"
#include "pretty_variable.hpp"
#include "xo/reflect/TypeDescr.hpp"
#include "xo/reflect/function/FunctionTdx.hpp"
#include "xo/indentlog/print/vector.hpp"
#include "xo/indentlog/print/pretty_vector.hpp"
#include <map>
#include <sstream>

namespace xo {
    using xo::reflect::TypeDescr;
    using xo::reflect::TypeDescrBase;
    using xo::reflect::FunctionTdxInfo;
    using std::stringstream;

    namespace scm {
        TypeDescr
        Lambda::assemble_lambda_td(const std::vector<rp<Variable>> & argv,
                                   TypeDescr return_td)
        {
            assert(return_td != nullptr);

            std::vector<TypeDescr> arg_td_v;
            {
                arg_td_v.reserve(argv.size());

                for (const auto & arg : argv) {
                    arg_td_v.push_back(arg->valuetype());
                }
            }

            auto function_info
                = FunctionTdxInfo(return_td,
                                  arg_td_v,
                                  false /*!is_noexcept*/);

            TypeDescr lambda_td
                = TypeDescrBase::require_by_fn_info(function_info);

            return lambda_td;
        }

        TypeDescr
        Lambda::assemble_lambda_td(const std::vector<rp<Variable>> & argv,
                                   TypeDescr explicit_return_td,
                                   const rp<Expression> & body)
        {
            if (!body)
                return nullptr;

            if (explicit_return_td && body->valuetype() && (explicit_return_td != body->valuetype())) {
                throw std::runtime_error(tostr("explicit lambda return type T1 conflicts with lambda body T2",
                                               xtag("T1", explicit_return_td),
                                               xtag("T2", body->valuetype())));
            }

            // TODO: unify(explicit_return_td, body->valuetype())

            TypeDescr return_td = explicit_return_td ? explicit_return_td : body->valuetype();

            return assemble_lambda_td(argv, return_td);
        }

        std::string
        Lambda::assemble_type_str(TypeDescr lambda_td) {
            assert(lambda_td);

            std::stringstream ss;

            ss << lambda_td->fn_retval()->short_name()
               << "(";

            for (std::size_t i = 0, n = lambda_td->n_fn_arg(); i < n; ++i) {
                if (i > 0)
                    ss << ",";
                ss << lambda_td->fn_arg(i)->short_name();
            }
            ss << ")";

            return ss.str();
        }

        rp<Lambda>
        Lambda::make(const std::string & name,
                     TypeDescr lambda_td,
                     const rp<LocalSymtab> & env,
                     const rp<Expression> & body)
        {
            return new Lambda(name, lambda_td, env, body);
        }

        rp<Lambda>
        Lambda::make_from_env(const std::string & name,
                              const rp<LocalSymtab> & env,
                              TypeDescr explicit_return_td,
                              const rp<Expression> & body)
        {
            TypeDescr lambda_td = assemble_lambda_td(env->argv(), explicit_return_td, body);

            rp<Lambda> retval
                = new Lambda(name,
                             lambda_td,
                             env,
                             body);

            /* need two-phase construction b/c pointer cycle */
            env->assign_origin(retval.get());

            return retval;
        }

        rp<Lambda>
        Lambda::make(const std::string & name,
                     const std::vector<rp<Variable>> & argv,
                     const rp<Expression> & body,
                     const rp<SymbolTable> & parent_env)
        {
            rp<LocalSymtab> env = LocalSymtab::make(argv, parent_env);

            TypeDescr explicit_return_td = nullptr;

            return make_from_env(name, env, explicit_return_td, body);
        } /*make*/

        std::set<std::string>
        Lambda::calc_free_variables() const
        {
            std::set<std::string> retval
                = body_->get_free_variables();

            /* but remove formals. */
            for (const auto & var : local_env_->argv())
                retval.erase(var->name());

            return retval;
        } /*calc_free_variables*/

        std::map<std::string, rp<Variable>>
        Lambda::regularize_layer_vars()
        {
            /* regularize local_env+body:  make sure exactly one instance
             * (i.e. with object identity) of a Variable appears
             * within one layer of a lambda body.
             *
             * Here 'layer' means excluding appearance in any nested lambdas
             * (i.e. whether or not such appearance would resolve to the same
             * memory location).
             *
             * Motivation is to unify Variables that would use the same
             * binding_path to resolve their runtime location.
             */
            std::map<std::string, rp<Variable>> var_map;

            for (const auto & arg : local_env_->argv()) {
                /* each arg name can appear at most once
                 * in a particular lambda's parameter list
                 */
                assert(var_map.find(arg->name()) == var_map.end());

                var_map[arg->name()] = arg;
            }

            this->body_
                = (body_->xform_layer
                   ([&var_map](bp<Expression> x) -> rp<Expression>
                    {
                        if (x->extype() == exprtype::variable) {
                            bp<Variable> var = Variable::from(x);

                            auto ix = var_map.find(var->name());
                            if (ix == var_map.end()) {
                                /* add to var_map,  copy to ensure Variable
                                 * not shared with any other layer
                                 */

                                var_map[var->name()] = Variable::copy(var);

                                return var.get();
                            } else {
                                /* substitute already-encountered var_map[] member */
                                return ix->second.get();
                            }
                        } else {
                            return x.get();
                        }
                    }));

            return var_map;
        } /*regularize_layer_vars*/

        void
        Lambda::complete_assembly_from_body() {
            if (body_) {
                TypeDescr explicit_return_td = nullptr;
                TypeDescr lambda_td
                    = assemble_lambda_td(this->local_env_->argv(), explicit_return_td, body_);

                if (lambda_td)
                    this->type_str_ = assemble_type_str(lambda_td);

                this->layer_var_map_ = this->regularize_layer_vars();

                this->free_var_set_ = this->calc_free_variables();

                std::map<std::string, bp<Lambda>> nested_lambda_map;
                {
                    this->body_->visit_layer
                        ([&nested_lambda_map]
                         (bp<Expression> expr)
                            {
                                if (expr->extype() == exprtype::lambda) {
                                    bp<Lambda> lm = Lambda::from(expr);

                                    nested_lambda_map[lm->name()] = lm.get();
                                }
                            });
                }
                this->nested_lambda_map_ = std::move(nested_lambda_map);

                /* establish the set of captured local vars.
                 * These are any formal parameters that appear free in
                 * any layer of a nested lambda.
                 */
                std::set<std::string> captured_var_set;
                {
                    for (const auto & ix : nested_lambda_map_) {
                        std::set<std::string> nested_free_var_set
                            = ix.second->get_free_variables();

                        for (const auto & jx : nested_free_var_set) {
                            /* check whether variable *jx is one of this lambda's
                             * formals
                             */
                            auto bind = this->local_env_->lookup_local_binding(jx);

                            if (bind.i_link_ == 0) {
                                /* yup,  it's a formal parameter of this lambda */
                                captured_var_set.insert(jx);
                            }
                        }
                    }
                }

                this->captured_var_set_ = std::move(captured_var_set);

                /* in particular:
                 * - establish binding path (intrusively) for each variable
                 *   assigns Variable::path_
                 */
                this->body_->attach_envs(local_env_);
            }
        }

        Lambda::Lambda(const std::string & name,
                       TypeDescr lambda_td,
                       const rp<LocalSymtab> & local_env,
                       const rp<Expression> & body)
            : FunctionExprInterface(exprtype::lambda, lambda_td),
              name_{name},
              body_{body},
              local_env_{local_env}
        {
#ifdef OBSOLETE
            stringstream ss;
            ss << "double";
            ss << "(";
            for (std::size_t i = 0, n = this->n_arg(); i < n; ++i) {
                if (i > 0)
                    ss << ",";
                ss << "double";
            }
            ss << ")";
#endif

            if (lambda_td)
                this->type_str_ = assemble_type_str(lambda_td);

            /* ensure variables are unique within layer for this lambda */
            this->layer_var_map_ = this->regularize_layer_vars();

            this->free_var_set_ = this->calc_free_variables();

            std::map<std::string, bp<Lambda>> nested_lambda_map;
            {
                this->body_->visit_layer
                    ([&nested_lambda_map]
                     (bp<Expression> expr)
                        {
                            if (expr->extype() == exprtype::lambda) {
                                bp<Lambda> lm = Lambda::from(expr);

                                nested_lambda_map[lm->name()] = lm.get();
                            }
                        });
            }
            this->nested_lambda_map_ = std::move(nested_lambda_map);

            /* establish the set of captured local vars.
             * These are any formal parameters that appear free in
             * any layer of a nested lambda.
             */
            std::set<std::string> captured_var_set;
            {
                for (const auto & ix : nested_lambda_map_) {
                    std::set<std::string> nested_free_var_set
                        = ix.second->get_free_variables();

                    for (const auto & jx : nested_free_var_set) {
                        /* check whether variable *jx is one of this lambda's
                         * formals
                         */
                        auto bind = this->local_env_->lookup_local_binding(jx);

                        if (bind.i_link_ == 0) {
                            /* yup,  it's a formal parameter of this lambda */
                            captured_var_set.insert(jx);
                        }
                    }
                }
            }

            this->captured_var_set_ = std::move(captured_var_set);

            /* in particular:
             * - establish binding path (intrusively) for each variable
             *   assigns Variable::path_
             */
            this->body_->attach_envs(local_env_);

        } /*ctor*/

        void
        Lambda::attach_envs(bp<SymbolTable> p) {
            local_env_->assign_parent(p);

            /** establish a binding path for each variable **/
        }

        void
        Lambda::display(std::ostream & os) const {
            os << "<Lambda"
               << xtag("name", name_)
               << xtag("argv", local_env_->argv())
               << xtag("body", body_)
               << ">";
        } /*display*/

        std::uint32_t
        Lambda::pretty_print(const ppindentinfo & ppii) const
        {
            return ppii.pps()->pretty_struct(ppii, "Lambda",
                                             refrtag("name", name_),
                                             refrtag("argv", local_env_->argv()),
                                             refrtag("body", body_));
        }

        // ----- Lambda Access -----

        rp<LambdaAccess>
        LambdaAccess::make(const std::string & name,
                           const std::vector<rp<Variable>> & argv,
                           const rp<Expression> & body,
                           const rp<SymbolTable> & parent_env)
        {
            TypeDescr explicit_return_td = nullptr;
            TypeDescr lambda_td = assemble_lambda_td(argv, explicit_return_td, body);
            rp<LocalSymtab> env = LocalSymtab::make(argv, parent_env);

            rp<LambdaAccess> retval
                = new LambdaAccess(name,
                                   lambda_td,
                                   env,
                                   body);

            /* need two-phase construction b/c pointer cycle */
            env->assign_origin(retval.get());

            return retval;
        }

        rp<LambdaAccess>
        LambdaAccess::make_empty()
        {
            return new LambdaAccess("" /*name*/,
                                    nullptr /*lambda_td*/,
                                    nullptr /*local_env*/,
                                    nullptr /*body*/);
        }

        LambdaAccess::LambdaAccess(const std::string & name,
                                   TypeDescr lambda_td,
                                   const rp<LocalSymtab> & local_env,
                                   const rp<Expression> & body)
            : Lambda(name, lambda_td, local_env, body)
        {}

        void
        LambdaAccess::assign_body(const rp<Expression> & body) {
            this->body_ = body;

            this->complete_assembly_from_body();
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end Lambda.cpp */
