/* file StructTdx.test.cpp
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
    TEST_CASE("std-pair-reflect", "[reflect]") {
      std::pair<int, double> p;

      TaggedPtr tp = Reflect::make_tp(&p);
      //TypeDescr td = Reflect::require<std::vector<double>>();

      REQUIRE(Reflect::is_reflected<std::pair<int, double>>() == true);

      REQUIRE(tp.td()->complete_flag());
      REQUIRE(tp.address() == &p);
      REQUIRE(tp.is_struct());
      REQUIRE(tp.is_vector() == false);
      REQUIRE(tp.td()->metatype() == Metatype::mt_struct);
      REQUIRE(tp.recover_native<std::pair<int, double>>() == &p);
      REQUIRE(tp.n_child() == 2); /* struct with 2 members */
      REQUIRE(tp.struct_member_name(0) == "first");
      REQUIRE(tp.struct_member_name(1) == "second");

      TaggedPtr tp0 = tp.get_child(0);

      REQUIRE(tp0.td()->complete_flag());
      REQUIRE(tp0.address() == &(p.first));
      REQUIRE(!tp0.is_vector());
      REQUIRE(!tp0.is_struct());
      REQUIRE(tp0.td()->metatype() == Metatype::mt_atomic);
      REQUIRE(tp0.recover_native<int>() == &(p.first));
      REQUIRE(tp0.n_child() == 0);

      TaggedPtr tp1 = tp.get_child(1);

      REQUIRE(tp1.td()->complete_flag());
      REQUIRE(tp1.address() == &(p.second));
      REQUIRE(!tp1.is_vector());
      REQUIRE(!tp1.is_struct());
      REQUIRE(tp1.td()->metatype() == Metatype::mt_atomic);
      REQUIRE(tp1.recover_native<double>() == &(p.second));
      REQUIRE(tp1.n_child() == 0);

    } /*TEST_CASE(std-pair-reflect)*/

  } /*namespace ut*/
} /*namespace xo*/

/* end VectorTdx.test.cpp */
