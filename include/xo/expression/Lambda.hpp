/** @file Lambda.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "Expression.hpp"
#include "Variable.hpp"
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

            const std::string & name() const { return name_; }
            const std::string & type_str() const { return type_str_; }
            const std::vector<ref::rp<Variable>> & argv() const { return argv_; }
            const ref::rp<Expression> & body() const { return body_; }

            /** return number of arguments expected by this function **/
            int n_arg() const { return argv_.size(); }

            // ----- Expression -----

            virtual void display(std::ostream & os) const override;

        private:
            /** @param lambda_type.  function type for this lambda.
             *  We arbitrarily choose the form "Retval(*)(Args...)"
             **/
            Lambda(const std::string & name,
                   TypeDescr lambda_type,
                   const std::vector<ref::rp<Variable>> & argv,
                   const ref::rp<Expression> & body);

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
            /** formal argument names **/
            std::vector<ref::rp<Variable>> argv_;
            /** function body **/
            ref::rp<Expression> body_;
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
