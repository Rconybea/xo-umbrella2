/** @file ex1.cpp **/

#include "xo/expression/Constant.hpp"
#include "xo/expression/Primitive.hpp"
#include "xo/expression/llvmintrinsic.hpp"
#include <iostream>
#include <cmath>
#include <math.h>

// address of &sqrt ambiguous on osx/clang
// (perhaps it's a template..?)
//
double xo_sqrt(double x) {
    return sqrt(x);
}

int
main() {
    using xo::ast::make_constant;
    using xo::ast::make_primitive;
    using xo::ast::llvmintrinsic;
    using std::cout;
    using std::endl;

    {
        auto expr = make_constant(7);
    }

    {
        auto expr = make_primitive("sqrt",
                                   &xo_sqrt,
                                   false /*!explicit_symbol_def*/,
                                   llvmintrinsic::fp_sqrt);

        auto expr_td = expr->value_td();

        cout << "expr_td: " << expr_td->short_name() << endl;
        cout << "expr_td->is_function(): " << expr_td->is_function() << endl;
        cout << "expr_td->fn_retval(): " << expr_td->fn_retval()->short_name() << endl;
        cout << "expr_td->n_fn_arg(): " << expr_td->n_fn_arg() << endl;
        for (uint32_t i = 0; i < expr_td->n_fn_arg(); ++i)
            cout << "expr_td->fn_arg(" << i << "): " << expr_td->fn_arg(i)->short_name() << endl;
        cout << "expr_td->fn_is_noexcept(): " << expr_td->fn_is_noexcept() << endl;
    }
}

/** end ex1.cpp **/
