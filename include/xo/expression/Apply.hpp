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
         *
         *  For first cut,  we'll just handle the case of a Constant
         *  that refers to a known function
         **/
        class Apply : public Expression {
            ref::rp<Expression> fn_;
            std::vector<ref::rp<Expression>> args_;
        };
    } /*namespace ast*/

} /*namespace xo*/


/** end Apply.hpp **/
