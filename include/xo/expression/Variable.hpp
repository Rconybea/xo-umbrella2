/** @file Variable.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "Expression.hpp"
#include "binding_path.hpp"

namespace xo {
    namespace ast {

        /** @class Variable
         *  @brief syntax for a variable reference
         **/
        class Variable : public Expression {
        public:
            /** create expression representing a variable
             *  identified by @p name,  that can take on values
             *  described by @p var_type.
             **/
            static ref::rp<Variable> make(const std::string & name,
                                          TypeDescr var_type) {
                return new Variable(name, var_type);
            }

            /** return copy of x:  same var, different object identity **/
            static ref::rp<Variable> copy(ref::brw<Variable> x) {
                return new Variable(x->name(), x->valuetype());
            }

            /** downcast from Expression **/
            static ref::brw<Variable> from(ref::brw<Expression> x) {
                return ref::brw<Variable>::from(x);
            }

            const std::string & name() const { return name_; }

            virtual std::set<std::string> get_free_variables() const override {
                std::set<std::string> retval;
                retval.insert(this->name_);
                return retval;
            }

            virtual std::size_t visit_preorder(VisitFn visitor_fn) override {
                visitor_fn(this);
                return 1;
            }

            virtual ref::rp<Expression> xform_layer(TransformFn xform_fn) override {
                return xform_fn(this);
            }

            virtual void attach_envs(ref::brw<Environment> /*p*/) override;

            virtual void display(std::ostream & os) const override;

        private:
            Variable(const std::string & name,
                     TypeDescr var_type)
                : Expression(exprtype::variable, var_type),
                  name_{name} {}

        private:
            /** variable name **/
            std::string name_;
            /** navigate environment via this path to find runtime memory
             *  location for this variable
             **/
            binding_path path_;
        }; /*Variable*/

        inline ref::rp<Variable>
        make_var(const std::string & name,
                 reflect::TypeDescr var_type) {
            return Variable::make(name, var_type);
        }
    } /*namespace ast*/
} /*namespace xo*/


/** end Variable.hpp **/
