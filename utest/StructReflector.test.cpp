/* file StructReflector.test.cpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#include "reflect/Reflect.hpp"
#include "reflect/StructReflector.hpp"
#include <catch2/catch.hpp>

#define STRINGIFY(x) #x
      
namespace xo {
  using xo::reflect::Reflect;
  using xo::reflect::TaggedPtr;
  using xo::reflect::StructReflector;
  using xo::reflect::Reflect;

  namespace ut {
    namespace {
      struct TestStruct0 {};
      struct TestStruct1 {};
    }

    TEST_CASE("struct-reflect-empty", "[reflect]") {
      StructReflector<TestStruct1> sr;

      REQUIRE(Reflect::is_reflected<TestStruct0>() == false);
      REQUIRE(Reflect::is_reflected<TestStruct1>() == true);

      TestStruct0 recd0;
      TaggedPtr tp = Reflect::make_tp(&recd0);

      REQUIRE(tp.address() == &recd0);
      REQUIRE(tp.td() == Reflect::require<TestStruct0>());

      REQUIRE(tp.n_child() == 0);

      REQUIRE(tp.get_child(0).is_universal_null());
      REQUIRE(tp.get_child(0).td() == nullptr);
      REQUIRE(tp.get_child(0).address() == nullptr);
    } /*TEST_CASE(struct-reflect-empty)*/

    namespace {
      struct TestStructS1 { int x_; };
    }

    TEST_CASE("struct-reflect-s1", "[reflect]") {
      StructReflector<TestStructS1> sr;

      REQUIRE(Reflect::is_reflected<TestStructS1>() == true);

      //sr.reflect_member(STRINGIFY(x_), &decltype(sr)::struct_t::x_);
      REFLECT_LITERAL_MEMBER(sr, x_);

      REQUIRE(!Reflect::require<TestStructS1>()->is_struct());

      sr.require_complete();

      REQUIRE(Reflect::require<TestStructS1>()->is_struct());
    } /*TEST_CASE(struct-reflect-s1)*/

    namespace {
      struct TestStructS2 { int x_; };
    }

    TEST_CASE("struct-reflect-s2", "[reflect]") {
      StructReflector<TestStructS2> sr;

      REQUIRE(Reflect::is_reflected<TestStructS2>() == true);

      //sr.reflect_member(STRINGIFY(x_), &decltype(sr)::struct_t::x_);
      REFLECT_MEMBER(sr, x);

      REQUIRE(!Reflect::require<TestStructS2>()->is_struct());

      sr.require_complete();

      REQUIRE(Reflect::require<TestStructS2>()->is_struct());

      TestStructS2 recd1{666};

      TaggedPtr tp = Reflect::make_tp(&recd1);

      REQUIRE(tp.address() == &recd1);
      REQUIRE(tp.td() == Reflect::require<TestStructS2>());

      REQUIRE(tp.n_child() == 1);

      REQUIRE(tp.get_child(0).td() == Reflect::require<int>());
      REQUIRE(tp.get_child(0).address() == &(recd1.x_));

      REQUIRE(tp.get_child(1).is_universal_null());
    } /*TEST_CASE(struct-reflect-s2)*/

    namespace {
      struct TestStructS3 { int x_; char y_; double z_; };
    }

    TEST_CASE("struct-reflect-s3", "[reflect]") {
      StructReflector<TestStructS3> sr;

      REQUIRE(Reflect::is_reflected<TestStructS3>() == true);

      REFLECT_MEMBER(sr, x);
      REFLECT_MEMBER(sr, y);
      REFLECT_MEMBER(sr, z);

      REQUIRE(!Reflect::require<TestStructS3>()->is_struct());

      sr.require_complete();

      REQUIRE(Reflect::require<TestStructS3>()->is_struct());

      /* verify we can traverse reflected instances */
      TestStructS3 recd1{666, 'Y', -1.234};

      TaggedPtr tp = Reflect::make_tp(&recd1);

      REQUIRE(tp.address() == &recd1);
      REQUIRE(tp.td() == Reflect::require<TestStructS3>());

      REQUIRE(tp.n_child() == 3);

      REQUIRE(tp.get_child(0).td() == Reflect::require<int>());
      REQUIRE(tp.get_child(0).address() == &(recd1.x_));

      REQUIRE(tp.get_child(1).td() == Reflect::require<char>());
      REQUIRE(tp.get_child(1).address() == &(recd1.y_));

      REQUIRE(tp.get_child(2).td() == Reflect::require<double>());
      REQUIRE(tp.get_child(2).address() == &(recd1.z_));
	      
      REQUIRE(tp.get_child(3).is_universal_null());
      REQUIRE(tp.get_child(3).td() == nullptr);
      REQUIRE(tp.get_child(3).address() == nullptr);

    } /*TEST_CASE(struct-reflect-s3)*/
  } /*namespace ut */
} /*namespace xo*/


/* end StructReflector.test.cpp */
