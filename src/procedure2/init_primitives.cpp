/** @file init_primitives.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "init_primitives.hpp"
#include "DPrimitive.hpp"
#include <xo/object2/Float.hpp>
//#include <xo/object2/number/IGCObject_DFloat.hpp>
#include <xo/object2/Integer.hpp>
//#include <xo/object2/number/IGCObject_DInteger.hpp>
#include <xo/object2/Boolean.hpp>
#include <xo/gc/GCObjectConversion.hpp>
#include <xo/object2/number/GCObjectConversion_DFloat.hpp>
#include <xo/object2/number/GCObjectConversion_DInteger.hpp>
#include <xo/facet/facet.hpp>
#include <cmath>

namespace xo {
    using xo::mm::AAllocator;
    using xo::scm::DFloat;
    using xo::facet::with_facet;

    namespace scm {
#ifdef NOT_YET
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
#endif

        obj<AGCObject>
        mul_gco_gco(obj<ARuntimeContext> rcx,
                    obj<AGCObject> x_gco,
                    obj<AGCObject> y_gco)
        {
            using xo::reflect::typeseq;

            obj<AAllocator> mm = rcx.allocator();

            // PLACEHOLDER

            // TODO:
            // 1. move this to xo-numeric2/ when available
            // 2. at that point will require polymorphic dispatch
            //    on argument representations, analogous to dispatch
            //    in FacetRegistry

            typeseq x_tseq = x_gco._typeseq();
            typeseq y_tseq = y_gco._typeseq();

            // FOR NOW: just test runtime values
            //
            if (x_tseq == typeseq::id<DInteger>()) {
                // i64 * ..
                long   x = GCObjectConversion<long>::from_gco(mm, x_gco);

                if (y_tseq == typeseq::id<DInteger>()) {
                    // i64 * i64
                    long   y = GCObjectConversion<long>::from_gco(mm, y_gco);

                    return DInteger::box<AGCObject>(mm, x * y);
                } else if (y_tseq == typeseq::id<DFloat>()) {
                    // i64 * f64
                    double y = GCObjectConversion<double>::from_gco(mm, y_gco);

                    return DFloat::box<AGCObject>(mm, x * y);
                }
            } else if (x_tseq == typeseq::id<DFloat>()) {
                if (y_tseq == typeseq::id<DInteger>()) {
                    // f64 * i64.
                    double x = GCObjectConversion<double>::from_gco(mm, x_gco);
                    long   y = GCObjectConversion<long>::from_gco(mm, y_gco);

                    return DFloat::box<AGCObject>(mm, x * y);
                } else if (y_tseq == typeseq::id<DFloat>()) {
                    // f64 * f64.
                    double x = GCObjectConversion<double>::from_gco(mm, x_gco);
                    double y = GCObjectConversion<double>::from_gco(mm, y_gco);

                    return DFloat::box<AGCObject>(mm, x * y);
                }
            }

            // here: error
            throw std::runtime_error(tostr("mul_gco_gco: unexpected argument types xt,yt",
                                           xtag("x.tseq", x_tseq),
                                           xtag("y.tseq", y_tseq)));
            return obj<AGCObject>();
        }

        obj<AGCObject>
        equal_gco_gco(obj<ARuntimeContext> rcx,
                      obj<AGCObject> x_gco,
                      obj<AGCObject> y_gco)
        {
            using xo::reflect::typeseq;

            obj<AAllocator> mm = rcx.allocator();

            // PLACEHOLDER

            // TODO
            // 1. move this to xo-numeric2/ when available
            // 2. at that point will require polymorphic dispatch on argument representations.
            // 

            typeseq x_tseq = x_gco._typeseq();
            typeseq y_tseq = y_gco._typeseq();

            // FOR NOW: just test runtime values
            //
            if (x_tseq == typeseq::id<DInteger>()) {
                // i64 * ..
                long   x = GCObjectConversion<long>::from_gco(mm, x_gco);

                if (y_tseq == typeseq::id<DInteger>()) {
                    // i64 == i64
                    long   y = GCObjectConversion<long>::from_gco(mm, y_gco);

                    return DBoolean::box<AGCObject>(mm, x == y);
                } else if (y_tseq == typeseq::id<DFloat>()) {
                    // i64 == f64
                    double y = GCObjectConversion<double>::from_gco(mm, y_gco);

                    return DFloat::box<AGCObject>(mm, static_cast<double>(x) == y);
                }
            } else if (x_tseq == typeseq::id<DFloat>()) {
                if (y_tseq == typeseq::id<DInteger>()) {
                    // f64 == i64.
                    double x = GCObjectConversion<double>::from_gco(mm, x_gco);
                    long   y = GCObjectConversion<long>::from_gco(mm, y_gco);

                    return DFloat::box<AGCObject>(mm, x == static_cast<double>(y));
                } else if (y_tseq == typeseq::id<DFloat>()) {
                    // f64 * f64.
                    double x = GCObjectConversion<double>::from_gco(mm, x_gco);
                    double y = GCObjectConversion<double>::from_gco(mm, y_gco);

                    return DFloat::box<AGCObject>(mm, x == y);
                }
            }
            
            // here: error
            throw std::runtime_error(tostr("mul_gco_gco: unexpected argument types xt,yt",
                                           xtag("x.tseq", x_tseq),
                                           xtag("y.tseq", y_tseq)));
            return obj<AGCObject>();
        }

#ifdef NOT_YET
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
#endif

        DPrimitive_gco_2_gco_gco
        Primitives::s_mul_gco_gco_pm("_mul", &mul_gco_gco);

        DPrimitive_gco_2_gco_gco
        Primitives::s_equal_gco_gco_pm("_equal", &equal_gco_gco);

#ifdef NOT_YET
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
#endif

    } /*namespace scm*/
} /*namespace xo*/

/* end init_primitives.cpp */
