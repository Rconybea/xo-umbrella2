/** @file DPrimitive.test.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include <xo/procedure2/init_procedure2.hpp>
#include <xo/procedure2/init_primitives.hpp>
#include <xo/procedure2/DSimpleRcx.hpp>
#include <xo/procedure2/detail/IRuntimeContext_DSimpleRcx.hpp>
#include <xo/procedure2/detail/IPrintable_DPrimitive_gco_2_gco_gco.hpp>
#include <xo/object2/DFloat.hpp>
#include <xo/object2/DInteger.hpp>
#include <xo/object2/DArray.hpp>
#include <xo/object2/number/IGCObject_DFloat.hpp>
#include <xo/object2/number/IGCObject_DInteger.hpp>
#include <xo/alloc2/arena/IAllocator_DArena.hpp>
#include <xo/printable2/Printable.hpp>
#include <xo/indentlog/scope.hpp>
#include <catch2/catch.hpp>
#include <sstream>

namespace xo {
    using xo::scm::Primitives;
    using xo::scm::DSimpleRcx;
    using xo::scm::ARuntimeContext;
    using xo::scm::DFloat;
    using xo::scm::DInteger;
    using xo::scm::DArray;
    using xo::scm::DPrimitive_gco_2_gco_gco;
    using xo::mm::AAllocator;
    using xo::mm::AGCObject;
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;
    using xo::print::APrintable;
    using xo::print::ppstate_standalone;
    using xo::print::ppconfig;
    using xo::facet::with_facet;
    using xo::facet::obj;
    using xo::scope;

    namespace ut {
        static InitEvidence s_init = InitSubsys<S_procedure2_tag>::require();

        TEST_CASE("DPrimitive-init", "[procedure2][DPrimitive]")
        {
            REQUIRE(s_init.evidence());
        }

        TEST_CASE("DPrimitive-n_args", "[procedure2][DPrimitive]")
        {
            // s_mul_gco_gco_pm takes 2 AGCObject args
            REQUIRE(Primitives::s_mul_gco_gco_pm.n_args() == 2);
        }

        TEST_CASE("DPrimitive-is_nary", "[procedure2][DPrimitive]")
        {
            REQUIRE(Primitives::s_mul_gco_gco_pm.is_nary() == false);
        }

        TEST_CASE("DPrimitive-apply_nocheck-float-float", "[procedure2][DPrimitive]")
        {
            ArenaConfig cfg { .name_ = "testarena", .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DSimpleRcx rcx_data(alloc);
            obj<ARuntimeContext> rcx = with_facet<ARuntimeContext>::mkobj(&rcx_data);

            // 3.0 * 7.0 = 21.0
            obj<AGCObject> x = DFloat::box<AGCObject>(alloc, 3.0);
            obj<AGCObject> y = DFloat::box<AGCObject>(alloc, 7.0);
            DArray * args = DArray::array(alloc, x, y);

            obj<AGCObject> result = Primitives::s_mul_gco_gco_pm.apply_nocheck(rcx, args);

            auto result_float = obj<AGCObject,DFloat>::from(result);
            REQUIRE(result_float);
            REQUIRE(result_float.data()->value() == 21.0);
        }

        TEST_CASE("DPrimitive-apply_nocheck-int-int", "[procedure2][DPrimitive]")
        {
            ArenaConfig cfg { .name_ = "testarena", .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DSimpleRcx rcx_data(alloc);
            obj<ARuntimeContext> rcx = with_facet<ARuntimeContext>::mkobj(&rcx_data);

            // 3 * 7 = 21
            obj<AGCObject> x = DInteger::box<AGCObject>(alloc, 3L);
            obj<AGCObject> y = DInteger::box<AGCObject>(alloc, 7L);
            DArray * args = DArray::array(alloc, x, y);

            obj<AGCObject> result = Primitives::s_mul_gco_gco_pm.apply_nocheck(rcx, args);

            auto result_int = obj<AGCObject,DInteger>::from(result);
            REQUIRE(result_int);
            REQUIRE(result_int.data()->value() == 21L);
        }

        TEST_CASE("DPrimitive-apply_nocheck-int-float", "[procedure2][DPrimitive]")
        {
            ArenaConfig cfg { .name_ = "testarena", .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DSimpleRcx rcx_data(alloc);
            obj<ARuntimeContext> rcx = with_facet<ARuntimeContext>::mkobj(&rcx_data);

            // 3 * 7.0 = 21.0 (mixed: result is float)
            obj<AGCObject> x = DInteger::box<AGCObject>(alloc, 3L);
            obj<AGCObject> y = DFloat::box<AGCObject>(alloc, 7.0);
            DArray * args = DArray::array(alloc, x, y);

            obj<AGCObject> result = Primitives::s_mul_gco_gco_pm.apply_nocheck(rcx, args);

            auto result_float = obj<AGCObject,DFloat>::from(result);
            REQUIRE(result_float);
            REQUIRE(result_float.data()->value() == 21.0);
        }

        TEST_CASE("DPrimitive-pretty", "[procedure2][DPrimitive][pp]")
        {
            scope log(XO_DEBUG(false));

            std::stringstream ss;
            ppconfig ppc;
            ppstate_standalone pps(&ss, 0, &ppc);

            obj<APrintable,DPrimitive_gco_2_gco_gco> prim_pr(&Primitives::s_mul_gco_gco_pm);
            pps.pretty(prim_pr);

            std::string output = ss.str();

            log && log(output);

            CHECK(output.find("_mul") != std::string::npos);
        }

    } /*namespace ut*/
} /*namespace xo*/

/* end DPrimitive.test.cpp */
