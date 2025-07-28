/* @file Primitive.cpp */

#include "Primitive.hpp"
#include <cstdint>

extern "C" {
    /** code here is used in two context:
     *  1. Fallback implementation under llvm.
     *     In practice will use llvm intrinsic instead.
     *     See xo-jit/src/jit/MachPipeline.cpp
     *  2. Schematika interpreter (aspirational asof jul 2025)
     *
     **/

    bool
    cmp_eq2_i64(std::int64_t x, std::int64_t y) {
        return x == y;
    }

    bool
    cmp_ne2_i64(std::int64_t x, std::int64_t y) {
        return x != y;
    }

    bool
    cmp_lt2_i64(std::int64_t x, std::int64_t y) {
        return x < y;
    }

    bool
    cmp_le2_i64(std::int64_t x, std::int64_t y) {
        return x <= y;
    }

    bool
    cmp_gt2_i64(std::int64_t x, std::int64_t y) {
        return x > y;
    }

    bool
    cmp_ge2_i64(std::int64_t x, std::int64_t y) {
        return x >= y;
    }

    std::int64_t
    add2_i64(std::int64_t x, std::int64_t y) {
        return x + y;
    }

    std::int64_t
    sub2_i64(std::int64_t x, std::int64_t y) {
        return x - y;
    }

    std::int64_t
    mul2_i64(std::int64_t x, std::int64_t y) {
        return x * y;
    }

    std::int64_t
    div2_i64(std::int64_t x, std::int64_t y) {
        return x / y;
    }

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
    namespace scm {
        auto
        Primitive_cmp_i64::make_cmp_eq2_i64() -> rp<PrimitiveType>
        {
            static rp<PrimitiveType> s_retval;

            if (!s_retval)
                s_retval = Primitive::make("cmp_eq2_i64",
                                           &cmp_eq2_i64,
                                           true /*explicit_symbol_def*/,
                                           llvmintrinsic::i_eq);

            return s_retval;
        }

        auto
        Primitive_cmp_i64::make_cmp_ne2_i64() -> rp<PrimitiveType>
        {
            static rp<PrimitiveType> s_retval;

            if (!s_retval)
                s_retval = Primitive::make("cmp_ne2_i64",
                                           &cmp_ne2_i64,
                                           true /*explicit_symbol_def*/,
                                           llvmintrinsic::i_ne);

            return s_retval;
        }

        auto
        Primitive_cmp_i64::make_cmp_lt2_i64() -> rp<PrimitiveType>
        {
            static rp<PrimitiveType> s_retval;

            if (!s_retval)
                s_retval = Primitive::make("cmp_lt2_i64",
                                           &cmp_lt2_i64,
                                           true /*explicit_symbol_def*/,
                                           llvmintrinsic::i_slt);

            return s_retval;
        }

        auto
        Primitive_cmp_i64::make_cmp_le2_i64() -> rp<PrimitiveType>
        {
            static rp<PrimitiveType> s_retval;

            if (!s_retval)
                s_retval = Primitive::make("cmp_le2_i64",
                                           &cmp_le2_i64,
                                           true /*explicit_symbol_def*/,
                                           llvmintrinsic::i_sle);

            return s_retval;
        }

        auto
        Primitive_cmp_i64::make_cmp_gt2_i64() -> rp<PrimitiveType>
        {
            static rp<PrimitiveType> s_retval;

            if (!s_retval)
                s_retval = Primitive::make("cmp_gt2_i64",
                                           &cmp_gt2_i64,
                                           true /*explicit_symbol_def*/,
                                           llvmintrinsic::i_sgt);

            return s_retval;
        }

        auto
        Primitive_cmp_i64::make_cmp_ge2_i64() -> rp<PrimitiveType>
        {
            static rp<PrimitiveType> s_retval;

            if (!s_retval)
                s_retval = Primitive::make("cmp_ge2_i64",
                                           &cmp_ge2_i64,
                                           true /*explicit_symbol_def*/,
                                           llvmintrinsic::i_sge);

            return s_retval;
        }

        /* TODO: remaining integer arithmetic */

        auto
        Primitive_i64::make_add2_i64() -> rp<PrimitiveType>
        {
            static rp<PrimitiveType> s_retval;

            if (!s_retval)
                s_retval = Primitive::make("add2_i64",
                                           &add2_i64,
                                           true /*explicit_symbol_def*/,
                                           llvmintrinsic::i_add);

            return s_retval;
        }

        auto
        Primitive_i64::make_sub2_i64() -> rp<PrimitiveType>
        {
            static rp<PrimitiveType> s_retval;

            if (!s_retval)
                s_retval = Primitive::make("sub2_i64",
                                           &sub2_i64,
                                           true /*explicit_symbol_def*/,
                                           llvmintrinsic::i_sub);

            return s_retval;
        }

        auto
        Primitive_i64::make_mul2_i64() -> rp<PrimitiveType>
        {
            static rp<PrimitiveType> s_retval;

            if (!s_retval)
                s_retval = Primitive::make("mul2_i64",
                                           &mul2_i64,
                                           true /*explicit_symbol_def*/,
                                           llvmintrinsic::i_mul);

            return s_retval;
        }

        auto
        Primitive_i64::make_div2_i64() -> rp<PrimitiveType>
        {
            static rp<PrimitiveType> s_retval;

            if (!s_retval)
                s_retval = Primitive::make("div2_i64",
                                           &div2_i64,
                                           true /*explicit_symbol+def*/,
                                           llvmintrinsic::i_sdiv);
            return s_retval;
        }

        // ----- floating-point arithmetic -----

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
