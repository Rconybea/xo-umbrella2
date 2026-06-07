/* file VectorTdx.test.cpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#include "xo/reflect/Reflect.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TypeDescr;
    using xo::reflect::Metatype;

    namespace ut {
        TEST_CASE("std-vector-reflect-empty", "[reflect]") {
            std::vector<double> v;

            TaggedPtr tp = Reflect::make_tp(&v);
            //TypeDescr td = Reflect::require<std::vector<double>>();

            REQUIRE(Reflect::is_reflected<std::vector<double>>() == true);

            REQUIRE(tp.td()->complete_flag());
            REQUIRE(tp.address() == &v);
            REQUIRE(tp.is_vector());
            REQUIRE(tp.is_struct() == false);
            REQUIRE(tp.td()->metatype() == Metatype::mt_vector);
            REQUIRE(tp.recover_native<std::vector<double>>() == &v);
            REQUIRE(tp.n_child() == 0); /*since empty vector*/
            // REQUIRE(tp.child_td(0) == ...
        } /*TEST_CASE(std-vector-reflect-empty)*/

        TEST_CASE("std-vector-reflect-one", "[reflect]") {
            std::vector<double> v = { 1.123 };

            TaggedPtr tp = Reflect::make_tp(&v);

            REQUIRE(Reflect::is_reflected<std::vector<double>>() == true);

            REQUIRE(tp.td()->complete_flag());
            REQUIRE(tp.address() == &v);
            REQUIRE(tp.is_vector());
            REQUIRE(tp.is_struct() == false);
            REQUIRE(tp.td()->metatype() == Metatype::mt_vector);
            REQUIRE(tp.recover_native<std::vector<double>>() == &v);
            REQUIRE(tp.n_child() == 1);

            TaggedPtr tp0 = tp.get_child(0);

            REQUIRE(tp0.td()->complete_flag());
            REQUIRE(tp0.address() == &(v[0]));
            REQUIRE(!tp0.is_vector());
            REQUIRE(!tp0.is_struct());
            REQUIRE(tp0.td()->metatype() == Metatype::mt_atomic);
            REQUIRE(tp0.recover_native<double>() == &(v[0]));
            REQUIRE(tp0.n_child() == 0);
        } /*TEST_CASE(std-vector-reflect-one)*/

        TEST_CASE("std-vector-reflect-two", "[reflect]") {
            std::vector<double> v = { 1.123, 2.234 };

            TaggedPtr tp = Reflect::make_tp(&v);

            REQUIRE(Reflect::is_reflected<std::vector<double>>() == true);

            REQUIRE(tp.td()->complete_flag());
            REQUIRE(tp.address() == &v);
            REQUIRE(tp.is_vector());
            REQUIRE(tp.is_struct() == false);
            REQUIRE(tp.td()->metatype() == Metatype::mt_vector);
            REQUIRE(tp.recover_native<std::vector<double>>() == &v);
            REQUIRE(tp.n_child() == 2);

            TaggedPtr tp0 = tp.get_child(0);

            REQUIRE(tp0.td()->complete_flag());
            REQUIRE(tp0.address() == &(v[0]));
            REQUIRE(!tp0.is_vector());
            REQUIRE(!tp0.is_struct());
            REQUIRE(tp0.td()->metatype() == Metatype::mt_atomic);
            REQUIRE(tp0.recover_native<double>() == &(v[0]));
            REQUIRE(tp0.n_child() == 0);

            TaggedPtr tp1 = tp.get_child(1);

            REQUIRE(tp1.td()->complete_flag());
            REQUIRE(tp1.address() == &(v[1]));
            REQUIRE(!tp1.is_vector());
            REQUIRE(!tp1.is_struct());
            REQUIRE(tp1.td()->metatype() == Metatype::mt_atomic);
            REQUIRE(tp1.recover_native<double>() == &(v[1]));
            REQUIRE(tp1.n_child() == 0);
        } /*TEST(std-vector-reflect-two)*/

        // ----- std::array -----

        TEST_CASE("std-array-reflect-empty", "[reflect]") {
            std::array<double, 0> v;

            TaggedPtr tp = Reflect::make_tp(&v);
            //TypeDescr td = Reflect::require<std::array<double, xx>>();

            REQUIRE(Reflect::is_reflected<std::array<double, 0>>() == true);

            REQUIRE(tp.td()->complete_flag());
            REQUIRE(tp.address() == &v);
            REQUIRE(tp.is_vector());
            REQUIRE(tp.is_struct() == false);
            REQUIRE(tp.td()->metatype() == Metatype::mt_vector);
            REQUIRE(tp.recover_native<std::array<double, 0>>() == &v);
            REQUIRE(tp.n_child() == 0); /*since empty vector*/
            // REQUIRE(tp.child_td(0) == ...
        } /*TEST_CASE(std-array-reflect-empty)*/

        TEST_CASE("std-array-reflect-one", "[reflect]") {
            std::array<double, 1> v = { 1.123 };

            TaggedPtr tp = Reflect::make_tp(&v);

            REQUIRE(Reflect::is_reflected<std::array<double, 1>>() == true);

            REQUIRE(tp.td()->complete_flag());
            REQUIRE(tp.address() == &v);
            REQUIRE(tp.is_vector());
            REQUIRE(tp.is_struct() == false);
            REQUIRE(tp.td()->metatype() == Metatype::mt_vector);
            REQUIRE(tp.recover_native<std::array<double, 1>>() == &v);
            REQUIRE(tp.n_child() == 1);

            TaggedPtr tp0 = tp.get_child(0);

            REQUIRE(tp0.td()->complete_flag());
            REQUIRE(tp0.address() == &(v[0]));
            REQUIRE(!tp0.is_vector());
            REQUIRE(!tp0.is_struct());
            REQUIRE(tp0.td()->metatype() == Metatype::mt_atomic);
            REQUIRE(tp0.recover_native<double>() == &(v[0]));
            REQUIRE(tp0.n_child() == 0);
        } /*TEST_CASE(std-array-reflect-one)*/

        TEST_CASE("std-array-reflect-two", "[reflect]") {
            std::array<double, 2> v = { 1.123, 2.234 };

            TaggedPtr tp = Reflect::make_tp(&v);

            REQUIRE(Reflect::is_reflected<std::array<double, 2>>() == true);

            REQUIRE(tp.td()->complete_flag());
            REQUIRE(tp.address() == &v);
            REQUIRE(tp.is_vector());
            REQUIRE(tp.is_struct() == false);
            REQUIRE(tp.td()->metatype() == Metatype::mt_vector);
            REQUIRE(tp.recover_native<std::array<double, 2>>() == &v);
            REQUIRE(tp.n_child() == 2);

            TaggedPtr tp0 = tp.get_child(0);

            REQUIRE(tp0.td()->complete_flag());
            REQUIRE(tp0.address() == &(v[0]));
            REQUIRE(!tp0.is_vector());
            REQUIRE(!tp0.is_struct());
            REQUIRE(tp0.td()->metatype() == Metatype::mt_atomic);
            REQUIRE(tp0.recover_native<double>() == &(v[0]));
            REQUIRE(tp0.n_child() == 0);

            TaggedPtr tp1 = tp.get_child(1);

            REQUIRE(tp1.td()->complete_flag());
            REQUIRE(tp1.address() == &(v[1]));
            REQUIRE(!tp1.is_vector());
            REQUIRE(!tp1.is_struct());
            REQUIRE(tp1.td()->metatype() == Metatype::mt_atomic);
            REQUIRE(tp1.recover_native<double>() == &(v[1]));
            REQUIRE(tp1.n_child() == 0);
        } /*TEST(std-array-reflect-two)*/

    } /*namespace ut*/
} /*namespace xo*/

/* end VectorTdx.test.cpp */
