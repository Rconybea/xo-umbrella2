/* file LocalEnv.hpp
 *
 * author: Roland Conybeare, Jun 2024
 */

#pragma once

#include "Environment.hpp"
#include "Variable.hpp"
#include "xo/reflect/TypeDescr.hpp"

namespace xo {
    namespace scm {
        class Lambda;

        /** @brief LocalEnv
         *
         *  @class Local environment for a lambda.
         *  Lists the Variables corresponding to this lambda's formal
         *  parameters,  but also links to @ref Environment for
         *  innermost enclosing @ref Lambda.
         **/
        class LocalEnv : public Environment {
        public:
            using TypeDescr = xo::reflect::TypeDescr;

        public:
            static rp<LocalEnv> make_empty();
            /** named ctor idiom.  Create instance with local variables per @p argv **/
            static rp<LocalEnv> make(const std::vector<rp<Variable>> & argv,
                                     const rp<Environment> & parent_env);
            /** Create instance with single local variable @ap argv1 **/
            static rp<LocalEnv> make1(const rp<Variable> & arg1,
                                      const rp<Environment> & parent_env);
            /** runtime downcast. nullptr if @p x is not a LocalEnv instance **/
            static bp<LocalEnv> from(const bp<Environment> & x) { return bp<LocalEnv>::from(x); }

            Lambda * origin() const { return origin_; }
            const std::vector<rp<Variable>> & argv() const { return argv_; }
            const rp<Variable>& lookup_arg(int i) const { return argv_[i]; }
            int n_arg() const { return argv_.size(); }
            TypeDescr fn_arg(uint32_t i) const { return argv_[i]->valuetype(); }

            /** report binding path for a formal parameter.
             *  Returns sentinel if @p vname doesn't appear in @ref argv_
             **/
            binding_path lookup_local_binding(const std::string & vname) const;

            /** single-assign this environment's origin **/
            void assign_origin(Lambda * p) {
                assert(origin_ == nullptr);
                origin_ = p;
            }

            /** single-assign this environment's parent **/
            void assign_parent(bp<Environment> p);

            // ----- Environment -----

            virtual bool is_global_env() const override { return false; }

            virtual binding_path lookup_binding(const std::string & vname) const override;

            virtual bp<Expression> lookup_var(const std::string & vname) const override {
                bp<Expression> retval = this->lookup_local(vname);

                if (retval)
                    return retval;

                /* here: target not found in local vars,
                 * delegate to innermost ancestor
                 */
                return parent_env_->lookup_var(vname);
            }

            virtual bp<Expression> lookup_local(const std::string & vname) const override {
                for (const auto & arg : argv_) {
                    if (arg->name() == vname)
                        return arg;
                }

                return bp<Expression>();
            }

            /** create/replace local variable @p target.
             *  Narrow use case: intended for when LocalEnv represents a top-level session environment.
             **/
            virtual void upsert_local(bp<Variable> target) override;

            virtual void print(std::ostream & os) const override;
            virtual std::uint32_t pretty_print(const print::ppindentinfo & ppii) const override;

        private:
            LocalEnv(const std::vector<rp<Variable>> & argv, const rp<Environment> & parent_env);

        private:
            /** Lambda for which this environment created.
             *
             *  Invariant:
             *  @code
             *  origin_->local_env_ == this
             *  @endcode
             **/
            Lambda * origin_ = nullptr;

            /** formal argument names **/
            std::vector<rp<Variable>> argv_;

            /** parent environment.  A free variable in this lambda's
             *  body will be resolved by referring them to @ref parent_env_.
             **/
            rp<Environment> parent_env_;
        };

    } /*namespace scm*/
} /*namespace xo*/


/* end LocalEnv.hpp */
