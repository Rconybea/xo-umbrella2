/* @file Primitive.cpp */

#include "Primitive.hpp"

extern "C" {
    double
    add2_f64(double x, double y) {
        return x + y;
    }

    double
    sub2_f64(double x, double y) {
        return x - y;
    }

    double
    mul2_f64(double x, double y) {
        return x * y;
    }

    double
    div2_f64(double x, double y) {
        return x / y;
    }
}

namespace xo {
    namespace ast {
        auto
        Primitive_f64::make_add2_f64() -> rp<PrimitiveType>
        {
            static rp<PrimitiveType> s_retval;

            if (!s_retval)
                s_retval = Primitive::make("add2_f64",
                                           &add2_f64,
                                           true /*explicit_symbol_def*/,
                                           llvmintrinsic::fp_add);

            return s_retval;
        }

        auto
        Primitive_f64::make_sub2_f64() -> rp<PrimitiveType>
        {
            static rp<PrimitiveType> s_retval;

            if (!s_retval)
                s_retval = Primitive::make("sub2_f64",
                                           &sub2_f64,
                                           true /*explicit_symbol_def*/,
                                           llvmintrinsic::fp_sub);

            return s_retval;
        }

        auto
        Primitive_f64::make_mul2_f64() -> rp<PrimitiveType>
        {
            static rp<PrimitiveType> s_retval;

            if (!s_retval)
                s_retval = Primitive::make("mul2_f64",
                                           &mul2_f64,
                                           true /*explicit_symbol_def*/,
                                           llvmintrinsic::fp_mul);

            return s_retval;
        }

        auto
        Primitive_f64::make_div2_f64() -> rp<PrimitiveType>
        {
            static rp<PrimitiveType> s_retval;

            if (!s_retval)
                s_retval = Primitive::make("div2_f64",
                                           &div2_f64,
                                           true /*explicit_symbol_def*/,
                                           llvmintrinsic::fp_div);

            return s_retval;
        }

    } /*namespace scm*/
} /*namespace xo*/


/* end Primitive.cpp */
