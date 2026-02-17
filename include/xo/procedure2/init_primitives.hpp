/** @file init_primitives.hpp **/

#pragma once

#include "DPrimitive.hpp"
#include "DPrimitive_gco_2_gco_gco.hpp"

namespace xo {
    namespace scm {
/** TODO: move this into xo-reader2 ? **/

#ifdef NOT_YET
        using Primitive_f64_1_f64 = Primitive<double (*)(double)>;
        using Primitive_f64_2_f64_f64 = Primitive<double (*)(double, double)>;
#endif

        struct Primitives {
            /** polymorphic multiply
             *
             *  TODO: this will want to move to xo-numeric/
             *        so we can dispatch on vector, matrix, function types
             **/
            static DPrimitive_gco_2_gco_gco s_mul_gco_gco_pm;

            /** polymorphic subtract
             *
             *  TODO: this will want to move to xo-numeric/
             *        so we can dispatch on vector, matrix, function types
             **/
            static DPrimitive_gco_2_gco_gco s_sub_gco_gco_pm;

            /** polymorphic equality comparison
             *
             *  TODO: this will want to move to x-numeric/
             **/
            static DPrimitive_gco_2_gco_gco s_equal_gco_gco_pm;

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
