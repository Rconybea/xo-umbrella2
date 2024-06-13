/** @file Variable.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "Expression.hpp"

namespace xo {
    namespace ast {

        /** @class Variable
         *  @brief syntax for a variable reference
         **/
        class Variable : public Expression {
        public:
            Variable(const std::string & name) : Expression(exprtype::variable), name_{name} {}

            /** downcast from Expression **/
            static ref::brw<Variable> from(ref::brw<Expression> x) {
                return ref::brw<Variable>::from(x);
            }

            const std::string & name() const { return name_; }

            virtual void display(std::ostream & os) const;

        private:
            /** variable name **/
            std::string name_;
        }; /*Variable*/

        inline ref::rp<Variable>
        make_var(const std::string & name) {
            return new Variable(name);
        }
    } /*namespace ast*/
} /*namespace xo*/


/** end Variable.hpp **/
