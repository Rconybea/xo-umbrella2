/** @file DString.test.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include <xo/object2/DString.hpp>
#include <xo/alloc2/arena/IAllocator_DArena.hpp>
#include <catch2/catch.hpp>
#include <cstring>

namespace xo {
    using xo::scm::DString;
    using xo::mm::AAllocator;
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;
    using xo::facet::with_facet;
    using xo::facet::obj;

    namespace ut {
        TEST_CASE("DString-empty", "[object2][DString]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DString * s = DString::empty(alloc, 16);

            REQUIRE(s != nullptr);
            REQUIRE(s->capacity() == 16);
            REQUIRE(s->size() == 0);
            REQUIRE(s->chars()[0] == '\0');
        }

        TEST_CASE("DString-from_cstr", "[object2][DString]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            const char * cstr = "hello world";
            DString * s = DString::from_cstr(alloc, cstr);

            REQUIRE(s != nullptr);
            REQUIRE(s->capacity() == 12);
            REQUIRE(s->size() == 11);
            REQUIRE(std::strcmp(s->chars(), cstr) == 0);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end DString.test.cpp */
