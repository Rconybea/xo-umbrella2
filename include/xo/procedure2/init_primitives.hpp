/** @file init_primitives.hpp **/

#pragma once

#include "DPrimitive.hpp"
#include "DPrimitive_gco_2_gco_gco.hpp"

namespace xo {
    namespace scm {
#ifdef NOT_YET
        using Primitive_f64_1_f64 = Primitive<double (*)(double)>;
        using Primitive_f64_2_f64_f64 = Primitive<double (*)(double, double)>;
#endif

        struct Primitives {
            static DPrimitive_gco_2_gco_gco s_mul_gco_gco_pm;

#ifdef NOT_YET
            static Primitive_f64_1_f64 s_neg_f64_pm;

            static Primitive_f64_2_f64_f64 s_add_f64_f64_pm;
            static Primitive_f64_2_f64_f64 s_sub_f64_f64_pm;
            static Primitive_f64_2_f64_f64 s_mul_f64_f64_pm;
            static Primitive_f64_2_f64_f64 s_div_f64_f64_pm;
            static Primitive_f64_2_f64_f64 s_pow_f64_f64_pm;

            static Primitive_f64_1_f64 s_log_f64_pm;
            static Primitive_f64_1_f64 s_sin_f64_pm;
            static Primitive_f64_1_f64 s_cos_f64_pm;
            static Primitive_f64_1_f64 s_tan_f64_pm;
#endif
        };
    }
}

/* end init_primitives.hpp */
