/** @file init_primitives.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "init_primitives.hpp"
#include "DPrimitive.hpp"

namespace xo {
    namespace scm {
        double
        neg_f64(double x) {
            return -x;
        }

        double
        add_f64_f64(double x, double y) {
            return x + y;
        }

        double
        sub_f64_f64(double x, double y) {
            return x - y;
        }

        double
        mul_f64_f64(double x, double y) {
            return x * y;
        }

        double
        div_f64_f64(double x, double y) {
            return x / y;
        }

        double
        pow_f64_f64(double x, double y) {
            return ::pow(x, y);
        }

        double
        log_f64(double x) {
            return ::log(x);
        }

        double
        sin_f64(double x) {
            return ::sin(x);
        }

        double
        cos_f64(double x) {
            return ::cos(x);
        }

        double
        tan_f64(double x) {
            return ::tan(x);
        }

        Primitive_f64_1_f64
        Primitives::s_neg_f64_pm("_neg_d",
                                 &neg_f64);

        Primitive_f64_2_f64_f64
        Primitives::s_add_f64_f64_pm("_add_d_d", &add_f64_f64);

        Primitive_f64_2_f64_f64
        Primitives::s_sub_f64_f64_pm("_sub_d_d", &sub_f64_f64);

        Primitive_f64_2_f64_f64
        Primitives::s_mul_f64_f64_pm("_mul_d_d", &mul_f64_f64);

        Primitive_f64_2_f64_f64
        Primitives::s_div_f64_f64_pm("_div_d_d", &div_f64_f64);

        Primitive_f64_2_f64_f64
        Primitives::s_pow_f64_f64_pm("_pow_d_d", &pow_f64_f64);

        Primitive_f64_1_f64
        Primitives::s_log_f64_pm("_log_d", &log_f64);

        Primitive_f64_1_f64
        Primitives::s_sin_f64_pm("_sin_d", &sin_f64);

        Primitive_f64_1_f64
        Primitives::s_cos_f64_pm("_cos_d", &cos_f64);

        Primitive_f64_1_f64
        Primitives::s_tan_f64_pm("_tan_d", &tan_f64);

    } /*namespace scm*/
} /*namespace xo*/

/* end init_primitives.cpp */
