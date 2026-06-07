/** @file ResourceVisitor.test.cpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#include <xo/testutil/Utest.hpp>
#include <xo/alloc2/ResourceVisitor.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::AResourceVisitor;

    namespace ut {

        TEST_CASE("ResourceVisitor-1", "[resourcevisitor]")
        {
            auto log = Utest::ut_scope();

            obj<AResourceVisitor> v;

            REQUIRE(v.iface());
            REQUIRE(!v.iface()->_has_null_vptr());
        }

    }
} /*namespace xo*/

/* end ResourceVisitor.test.cpp */
