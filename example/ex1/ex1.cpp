/** @file ex1.cpp **/

#include "xo/expression/Constant.hpp"
#include <iostream>

int
main() {
    using xo::ast::make_constant;

    auto expr = make_constant(7);
}


/** end ex1.cpp **/
