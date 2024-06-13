/** @file Apply.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "Expression.hpp"

//#include <cstdint>

namespace xo {
    namespace ast {

        /** @class Apply
         *  @brief syntax for a function call.
         *
         *  In general we don't know function to be invoked
         *  until runtime,  depending on the nature of Expression.
         **/
        class Apply : public Expression {
        public:
            Apply(const ref::rp<Expression> & fn,
                  const std::vector<ref::rp<Expression>> & argv)
                : Expression(exprtype::apply), fn_{fn}, argv_(argv)
                {}

            /** downcast from Expression **/
            static ref::brw<Apply> from(ref::brw<Expression> x) {
                return ref::brw<Apply>::from(x);
            }

            const ref::rp<Expression> & fn() const { return fn_; }
            const std::vector<ref::rp<Expression>> & argv() const { return argv_; }

            virtual void display(std::ostream & os) const;

        private:
            /** function to invoke **/
            ref::rp<Expression> fn_;
            /** argument expressions,  in l-to-r order **/
            std::vector<ref::rp<Expression>> argv_;
        }; /*Apply*/

        inline ref::rp<Apply>
        make_apply(const ref::rp<Expression> & fn,
                   const ref::rp<Expression> & arg1) {
            std::vector<ref::rp<Expression>> argv;
            argv.push_back(arg1);

            return new Apply(fn, argv);
        } /*make_apply*/

    } /*namespace ast*/
} /*namespace xo*/


/** end Apply.hpp **/
