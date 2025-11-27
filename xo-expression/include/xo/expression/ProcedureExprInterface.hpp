/** @file FunctionExprInterface.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "Expression.hpp"
//#include <cstdint>

namespace xo {
    namespace scm {
        class ProcedureExprInterface : public Expression {
        public:
            ProcedureExprInterface(exprtype extype, TypeDescr fn_type)
                : Expression(extype, fn_type) {}

            /** downcast from Expression **/
            static bp<ProcedureExprInterface> from(bp<Expression> x) {
                return bp<ProcedureExprInterface>::from(x);
            }

            virtual const std::string & name() const = 0;
            virtual int n_arg() const = 0; // { return this->value_td()->n_fn_arg(); }
            virtual TypeDescr fn_retval() const = 0; // { return this->value_td()->fn_retval(); }
            virtual TypeDescr fn_arg(uint32_t i) const = 0; // { return this->value_td()->fn_arg(i); }

        private:
        }; /*FunctionInterface*/
    } /*namespace scm*/
} /*namespace xo*/

/** end FunctionExprInterface.hpp **/
