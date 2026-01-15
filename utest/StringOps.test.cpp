/** @file StringOps.test.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include <xo/object2/StringOps.hpp>
#include <xo/alloc2/arena/IAllocator_DArena.hpp>
#include <catch2/catch.hpp>
#include <cstring>

namespace xo {
    using xo::scm::StringOps;
    using xo::scm::DString;
    using xo::mm::AAllocator;
    using xo::mm::AGCObject;
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;
    using xo::facet::with_facet;
    using xo::facet::obj;

    namespace ut {
        TEST_CASE("StringOps-empty", "[object2][StringOps]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            auto s = StringOps::empty(alloc, 16);

            REQUIRE(s.data() != nullptr);
            REQUIRE(s.data()->capacity() == 16);
            REQUIRE(s.data()->size() == 0);
            REQUIRE(s.data()->chars()[0] == '\0');
        }

        TEST_CASE("StringOps-empty-with-content", "[object2][StringOps]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            auto s = StringOps::empty(alloc, 32);

            s.data()->sprintf("hello %s %d", "world", 42);

            REQUIRE(s.data()->size() == 14);
            REQUIRE(std::strcmp(s.data()->chars(), "hello world 42") == 0);
        }

        TEST_CASE("StringOps-from_cstr", "[object2][StringOps]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            const char * cstr = "hello world";
            auto s = StringOps::from_cstr(alloc, cstr);

            REQUIRE(s.data() != nullptr);
            REQUIRE(s.data()->capacity() == 12);
            REQUIRE(s.data()->size() == 11);
            REQUIRE(std::strcmp(s.data()->chars(), cstr) == 0);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end StringOps.test.cpp */
