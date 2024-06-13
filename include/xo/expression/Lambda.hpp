/** @file Lambda.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "Expression.hpp"
#include <vector>
#include <string>
//#include <cstdint>

namespace xo {
    namespace ast {
        /** @class Lambda
         *  @brief abstract syntax tree for a function definition
         *
         **/
        class Lambda : public Expression {
        public:
            /** @p argv   Formal parameters, in left-to-right order
             *  @p body   Expression for body of this function
             **/
            Lambda(const std::vector<std::string> & argv,
                   const ref::rp<Expression> & body)
                : Expression(exprtype::lambda), argv_{argv}, body_{body} {}

            const std::vector<std::string> & argv() const { return argv_; }
            const ref::rp<Expression> & body() const { return body_; }

            // ----- Expression -----

            virtual void display(std::ostream & os) const override;

        private:
            /** formal argument names **/
            std::vector<std::string> argv_;
            /** function body **/
            ref::rp<Expression> body_;
        }; /*Lambda*/
    } /*namespace ast*/
} /*namespace xo*/

/** end Lambda.hpp **/
