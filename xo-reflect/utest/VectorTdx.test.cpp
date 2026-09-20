/* file VectorTdx.test.cpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#include "xo/reflect/Reflect.hpp"
#include <xo/arena/DArenaVector.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TypeDescr;
    using xo::reflect::Metatype;
    using xo::mm::ArenaConfig;
    using xo::mm::ArenaNameStr;
    using xo::mm::DArenaVector;

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

        namespace {
            /** an arena-backed vector, sized so the tests below never grow
             *  into a second commit.  A DArenaVector fixes capacity at
             *  construction, which is the interesting difference from
             *  std::vector as far as reflection is concerned: n_child()
             *  reports size(), never capacity
             **/
            template <typename T>
            DArenaVector<T> make_vec(const char * name) {
                return DArenaVector<T>::map(ArenaConfig()
                                            .with_name(ArenaNameStr::from_cstr(name))
                                            .with_size(64*1024));
            }

            /** a struct element, to check the ELEMENT type is established and
             *  not just the container
             **/
            struct DVecProbe {
                double x_;
                double y_;
            };
        }

        /* DArenaVector reflects through StlVectorTdx, which is generic over
         * the container rather than tied to std::vector.  Before
         * .xo-backlog/xo-reflect/issues/02 there was no EstablishTdx
         * specialisation at all, so it fell to the primary template and
         * reflected as an ATOM -- which is what `mt_vector' below pins.
         */
        TEST_CASE("darena-vector-reflect-empty", "[reflect][darenavector]") {
            auto v = make_vec<double>("utest.dav.empty");

            TaggedPtr tp = Reflect::make_tp(&v);

            REQUIRE(Reflect::is_reflected<DArenaVector<double>>() == true);

            REQUIRE(tp.td()->complete_flag());
            REQUIRE(tp.address() == &v);
            REQUIRE(tp.is_vector());
            REQUIRE(tp.is_struct() == false);
            REQUIRE(tp.td()->metatype() == Metatype::mt_vector);
            REQUIRE(tp.recover_native<DArenaVector<double>>() == &v);
            REQUIRE(tp.n_child() == 0);

            /* capacity is large and non-zero; n_child tracks SIZE */
            REQUIRE(v.capacity() > 0);
        } /*TEST_CASE(darena-vector-reflect-empty)*/

        TEST_CASE("darena-vector-reflect-two", "[reflect][darenavector]") {
            auto v = make_vec<double>("utest.dav.two");

            v.push_back(1.125);
            v.push_back(2.25);

            TaggedPtr tp = Reflect::make_tp(&v);

            REQUIRE(tp.td()->metatype() == Metatype::mt_vector);
            REQUIRE(tp.n_child() == 2);

            TaggedPtr tp0 = tp.get_child(0);

            REQUIRE(tp0.td()->complete_flag());
            REQUIRE(tp0.address() == &(v[0]));
            REQUIRE(tp0.td()->metatype() == Metatype::mt_atomic);
            REQUIRE(tp0.recover_native<double>() == &(v[0]));
            REQUIRE(*tp0.recover_native<double>() == 1.125);

            TaggedPtr tp1 = tp.get_child(1);

            REQUIRE(tp1.address() == &(v[1]));
            REQUIRE(*tp1.recover_native<double>() == 2.25);

            /* elements are addressed through operator[] -> _address_of, which
             * is preamble-aware.  If that regressed, element 0 would land on
             * the arena back pointer -- see .xo-backlog/xo-arena/issues/05
             */
            REQUIRE(static_cast<void *>(&v[0]) != static_cast<void *>(&v[1]));
        } /*TEST_CASE(darena-vector-reflect-two)*/

        TEST_CASE("darena-vector-establishes-its-element-type",
                  "[reflect][darenavector]") {
            /* the specialisation calls Reflect::require<Element>() before
             * building the Tdx, so the ELEMENT is described even when nothing
             * else in the program mentions it
             */
            auto v = make_vec<DVecProbe>("utest.dav.struct");

            v.push_back(DVecProbe{3.0, 4.0});

            TaggedPtr tp = Reflect::make_tp(&v);

            REQUIRE(tp.td()->metatype() == Metatype::mt_vector);
            REQUIRE(tp.n_child() == 1);

            REQUIRE(tp.td()->fixed_child_td(0) == Reflect::require<DVecProbe>());

            TaggedPtr tp0 = tp.get_child(0);

            REQUIRE(tp0.recover_native<DVecProbe>() == &(v[0]));
            REQUIRE(tp0.recover_native<DVecProbe>()->y_ == 4.0);
        } /*TEST_CASE(darena-vector-establishes-its-element-type)*/

    } /*namespace ut*/
} /*namespace xo*/

/* end VectorTdx.test.cpp */
