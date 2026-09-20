/* file PrintJson.test.cpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#include "xo/printjson/PrintJson.hpp"
#include "xo/printjson/init_printjson.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/StructReflector.hpp>
#include <xo/ppsink/tag_ostream.hpp>   /* os << tag(..) */
#include <xo/arena/DArenaVector.hpp>
#include <catch2/catch.hpp>
#include <iostream>
#include <sstream>

//#define STRINGIFY(x) #x

namespace xo {
    using xo::json::PrintJson;
    using xo::reflect::Reflect;
    using xo::reflect::StructReflector;
    using xo::reflect::TaggedPtr;

    namespace ut {
        /* one scope in from namespace xo: a using-decl at xo scope would be
         * ambiguous with legacy xo::tag rather than shadowing it.
         */
        using xo::pp::tag;

        InitEvidence s_init_evidence = InitSubsys<S_printjson_tag>::require();

        namespace {
            struct TestStruct0 {};

            /** arena-backed vector, for the DArenaVector cases below **/
            template <typename T>
            xo::mm::DArenaVector<T> make_arena_vec(const char * name) {
                using xo::mm::ArenaConfig;
                using xo::mm::ArenaNameStr;

                return xo::mm::DArenaVector<T>::map(ArenaConfig()
                                                    .with_name(ArenaNameStr::from_cstr(name))
                                                    .with_size(64*1024));
            }
        }

        /* DArenaVector reflects as mt_vector since
         * .xo-backlog/xo-reflect/issues/02.  The point of these two cases is
         * that printjson needed NO change to render it: print_generic_vector
         * keys on the metatype, not on std::vector, so describing the
         * container was the whole of the work.
         *
         * Before that specialisation existed, DArenaVector fell to
         * EstablishTdx's primary template and reflected as an atom, so this
         * rendered <error-json-printer-not-found ... metatype=mt_atomic>.
         */
        TEST_CASE("print-json-darena-vector", "[printjson][darenavector]") {
            PrintJson print_json;

            auto v = make_arena_vec<double>("utest.pj.dav");

            v.push_back(1.5);
            v.push_back(2.25);
            v.push_back(-3.0);

            std::stringstream ss;
            print_json.print(v, &ss);

            INFO("rendered: " << ss.str());

            REQUIRE(ss.str() == std::string("[1.5, 2.25, -3]"));
        } /*TEST_CASE(print-json-darena-vector)*/

        TEST_CASE("print-json-darena-vector-empty", "[printjson][darenavector]") {
            PrintJson print_json;

            auto v = make_arena_vec<double>("utest.pj.dav.empty");

            std::stringstream ss;
            print_json.print(v, &ss);

            /* empty, not null: n_child() reports SIZE, and a DArenaVector's
             * capacity is fixed at construction, so a non-zero capacity must
             * not show up here
             */
            REQUIRE(ss.str() == std::string("[]"));
        } /*TEST_CASE(print-json-darena-vector-empty)*/

        TEST_CASE("print-json-empty-struct", "[printjson]") {
            INFO(tag("s_init_evidence", s_init_evidence));

            StructReflector<TestStruct0> sr;

            sr.require_complete();

            TestStruct0 recd0;

            PrintJson print_json;

            TaggedPtr tp = Reflect::make_tp(&recd0);

            std::stringstream ss;

            print_json.print(tp, &ss);

            REQUIRE(ss.str() == std::string("{\"_name_\": \"TestStruct0\"}"));
        } /*TEST_CASE(print-json-empty-struct)*/

        namespace {
            struct TestStruct1 {
                std::int16_t i16_; std::uint16_t u16_;
                std::int32_t i32_; std::uint32_t u32_;
                std::int64_t i64_; std::uint64_t u64_;
                float f32_; double f64_;
                std::string s_;
            };
        }

        TEST_CASE("print-json-s1", "[printjson]") {
            INFO(tag("s_init_evidence", s_init_evidence));

            StructReflector<TestStruct1> sr;
            {
                REFLECT_MEMBER(sr, i16);
                REFLECT_MEMBER(sr, u16);
                REFLECT_MEMBER(sr, i32);
                REFLECT_MEMBER(sr, u32);
                REFLECT_MEMBER(sr, i64);
                REFLECT_MEMBER(sr, u64);
                REFLECT_MEMBER(sr, f32);
                REFLECT_MEMBER(sr, f64);
                REFLECT_MEMBER(sr, s);

                sr.require_complete();
            }

            TestStruct1 recd1{-1, 2, -3, 4, -5, 6, 1.23f, 4.56, "hello, world"};

            PrintJson print_json;

            TaggedPtr tp = Reflect::make_tp(&recd1);

            std::stringstream ss;

            print_json.print(tp, &ss);

            REQUIRE(ss.str() == std::string("{\"_name_\": \"TestStruct1\""
                                            ", \"i16\": -1"
                                            ", \"u16\": 2"
                                            ", \"i32\": -3"
                                            ", \"u32\": 4"
                                            ", \"i64\": -5"
                                            ", \"u64\": 6"
                                            ", \"f32\": 1.23"
                                            ", \"f64\": 4.56"
                                            ", \"s\": \"hello, world\"}"));
        } /*TEST_CASE(print-json-s1)*/

        TEST_CASE("print-json-v1", "[printjson]") {
            INFO(tag("s_init_evidence", s_init_evidence));

            std::vector<double> v1{1, 2, 3};

            PrintJson print_json;

            TaggedPtr tp = Reflect::make_tp(&v1);

            std::stringstream ss;

            print_json.print(tp, &ss);

            REQUIRE(ss.str() == std::string("[1, 2, 3]"));
        } /*TEST_CASE(print-json-v1)*/

        /* also see tests:
         *   [option_util/utest/Px2.test.cpp]
         *   [option_util/utest/Size2.test.cpp]
         *   [option_util/utest/PxSize2.test.cpp]
         */
    } /*namespace ut */
} /*namespace xo*/


/* end StructReflector.test.cpp */
