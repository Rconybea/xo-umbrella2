/** @file Variable.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "Expression.hpp"
#include "binding_path.hpp"

namespace xo {
    namespace scm {

        /** @class Variable
         *  @brief syntax for a variable reference
         **/
        class Variable : public Expression {
        public:
            /** Generate unique symbol-name beginning with @p prefix.
             *  Relies on static counter
             **/
            static std::string gensym(const std::string & prefix);

            /** create expression representing a variable
             *  identified by @p name,  that can take on values
             *  described by @p var_type.
             **/
            static rp<Variable> make(const std::string & name,
                                     TypeDescr var_type) {
                return new Variable(name, var_type);
            }

            /** return copy of @p x:  same var, different object identity **/
            static rp<Variable> copy(bp<Variable> x) {
                return new Variable(x->name(), x->valuetype());
            }

            /** downcast from Expression **/
            static bp<Variable> from(bp<Expression> x) {
                return bp<Variable>::from(x);
            }

            void assign_name(const std::string & name) { name_ = name; }

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

            virtual std::size_t visit_layer(VisitFn visitor_fn) override {
                visitor_fn(this);
                return 1;
            }

            virtual rp<Expression> xform_layer(TransformFn xform_fn) override {
                return xform_fn(this);
            }

            virtual void attach_envs(bp<Environment> /*p*/) override;

            virtual void display(std::ostream & os) const override;
            virtual std::uint32_t pretty_print(const ppindentinfo & ppii) const override;

        private:
            Variable(const std::string & name,
                     TypeDescr var_type)
                : Expression(exprtype::variable, var_type),
                  name_{name} {}

        private:
            /** variable name **/
            std::string name_;
            /** Eventually: navigate environment via this path to find runtime memory
              * location for this variable.
              *
              * Establish via @ref attach_envs
             **/
            binding_path path_;
        }; /*Variable*/

        inline rp<Variable>
        make_var(const std::string & name,
                 reflect::TypeDescr var_type) {
            return Variable::make(name, var_type);
        }
    } /*namespace scm*/
} /*namespace xo*/


/** end Variable.hpp **/
