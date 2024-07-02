/* file LocalEnv.hpp
 *
 * author: Roland Conybeare, Jun 2024
 */

#pragma once

#include "Environment.hpp"

namespace xo {
    namespace ast {
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
            /** named ctor idiom.  Create instance with local variables per @p argv **/
            static ref::rp<LocalEnv> make(const std::vector<ref::rp<Variable>> & argv) {
                return new LocalEnv(argv);
            }

            Lambda * origin() const { return origin_; }
            const std::vector<ref::rp<Variable>> & argv() const { return argv_; }
            int n_arg() const { return argv_.size(); }
            TypeDescr fn_arg(uint32_t i) const { return argv_[i]->valuetype(); }

            /** single-assign this environment's origin **/
            void assign_origin(Lambda * p) {
                assert(origin_ == nullptr);
                origin_ = p;
            }

            /** single-assign this environment's parent **/
            void assign_parent(ref::brw<Environment> p) {
                assert(parent_env_.get() == nullptr);
                parent_env_ = p.get();
            }

            // ----- Environment -----

            virtual bool is_global_env() const override { return false; }

            virtual binding_path lookup_binding(const std::string & vname) const override;

            virtual ref::brw<Expression> lookup_var(const std::string & target) const override {
                for (const auto & arg : argv_) {
                    if (arg->name() == target)
                        return arg;
                }

                /* here: target not found in local vars,
                 * delegate to innermost ancestor
                 */
                return parent_env_->lookup_var(target);
            }

        private:
            LocalEnv(const std::vector<ref::rp<Variable>> & argv)
                : origin_{nullptr}, argv_(argv) {}

        private:
            /** Lambnda for which this environment created.
             *
             *  Invariant:
             *  @code
             *  origin_->local_env_ == this
             *  @endcode
             **/
            Lambda * origin_ = nullptr;

            /** formal argument names **/
            std::vector<ref::rp<Variable>> argv_;

            /** parent environment.  A free variable in this lambda's
             *  body will be resolved by referring them to @ref parent_env_.
             **/
            ref::rp<Environment> parent_env_;
        };
    } /*namespace ast*/
} /*namespace xo*/


/* end LocalEnv.hpp */
