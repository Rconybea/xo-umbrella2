/* @file ratio_reflect.test.cpp */

#include "xo/ratio/ratio_reflect.hpp"
#include "xo/reflect/reflect_struct.hpp"
#include <catch2/catch.hpp>

namespace xo {
    namespace ut {
        TEST_CASE("ratio_reflect", "[ratio][reflect]") {
            using xo::reflect::reflect_struct;
            using xo::reflect::TypeDescrBase;
            using xo::reflect::TypeDescr;
            using xo::ratio::ratio;

            /* verify ratio reflection */

            TypeDescr td = reflect_struct<ratio<std::int64_t>>();

            REQUIRE(td->is_struct());
            REQUIRE(td->metatype() == xo::reflect::Metatype::mt_struct);
            REQUIRE(td->n_child(nullptr) == 2);
            REQUIRE(td->struct_member(0).member_name() == "num");
            REQUIRE(td->struct_member(0).get_member_td()->short_name() == "long int");
            REQUIRE(td->struct_member(1).member_name() == "den");
            REQUIRE(td->struct_member(1).get_member_td()->short_name() == "long int");

            TypeDescrBase::print_reflected_types(std::cerr);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end ratio_reflect.test.cpp */
