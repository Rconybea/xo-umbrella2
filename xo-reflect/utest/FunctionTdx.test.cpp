/* @file FunctionTdx.test.cpp */

#include "xo/reflect/Reflect.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TypeDescr;
    using xo::reflect::Metatype;

    namespace ut {
        TEST_CASE("function-reflect1", "[reflect]") {
            using FunctionType = double (*)(double);

            FunctionType fn = ::sqrt;

            TaggedPtr tp = Reflect::make_tp(&fn);
            //TypeDescr td = Reflect::require<std::vector<double>>();

            REQUIRE(Reflect::is_reflected<FunctionType>() == true);

            REQUIRE(tp.td()->complete_flag());
            REQUIRE(tp.address() == &fn);
            REQUIRE(tp.is_function());
            REQUIRE(tp.is_pointer() == false);
            REQUIRE(tp.is_vector() == false);
            REQUIRE(tp.is_struct() == false);
            REQUIRE(tp.td()->metatype() == Metatype::mt_function);
            REQUIRE(tp.recover_native<double (*)(double)>() == &fn);
            REQUIRE(tp.n_child() == 0); /*not a composite*/
            // REQUIRE(tp.child_td(0) == ...
            REQUIRE(tp.td()->fn_retval() == Reflect::require<double>());
            REQUIRE(tp.n_fn_arg() == 1);
            REQUIRE(tp.td()->fn_arg(0) == Reflect::require<double>());
        } /*TEST_CASE(function-reflect1)*/
    } /*namespace ut*/
} /*namespace xo*/

/* end FunctionTdx.test.cpp */
