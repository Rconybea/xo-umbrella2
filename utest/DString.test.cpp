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

        TEST_CASE("DString-assign", "[object2][DString]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DString * src = DString::from_cstr(alloc, "hello");
            DString * dst = DString::empty(alloc, 16);

            dst->assign(*src);

            REQUIRE(dst->size() == 5);
            REQUIRE(std::strcmp(dst->chars(), "hello") == 0);
        }

        TEST_CASE("DString-assign-truncate", "[object2][DString]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DString * src = DString::from_cstr(alloc, "hello world");
            DString * dst = DString::empty(alloc, 6);

            dst->assign(*src);

            REQUIRE(dst->size() == 5);
            REQUIRE(std::strcmp(dst->chars(), "hello") == 0);
        }

        TEST_CASE("DString-data", "[object2][DString]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DString * s = DString::empty(alloc, 16);
            char * p = s->data();
            std::strcpy(p, "test");
            s->fixup_size();

            REQUIRE(s->size() == 4);
            REQUIRE(std::strcmp(s->chars(), "test") == 0);
        }

        TEST_CASE("DString-operator-bracket", "[object2][DString]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DString * s = DString::from_cstr(alloc, "hello");

            REQUIRE((*s)[0] == 'h');
            REQUIRE((*s)[4] == 'o');

            (*s)[0] = 'H';
            REQUIRE(std::strcmp(s->chars(), "Hello") == 0);
        }

        TEST_CASE("DString-clear", "[object2][DString]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DString * s = DString::from_cstr(alloc, "hello");
            REQUIRE(s->size() == 5);

            s->clear();

            REQUIRE(s->size() == 0);
            REQUIRE(s->chars()[0] == '\0');
            REQUIRE(s->capacity() == 6);
        }

        TEST_CASE("DString-fixup_size", "[object2][DString]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DString * s = DString::empty(alloc, 16);
            char * p = s->data();
            p[0] = 'a';
            p[1] = 'b';
            p[2] = 'c';
            p[3] = '\0';

            REQUIRE(s->size() == 0);

            auto new_size = s->fixup_size();

            REQUIRE(new_size == 3);
            REQUIRE(s->size() == 3);
        }

        TEST_CASE("DString-string_view", "[object2][DString]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DString * s = DString::from_cstr(alloc, "hello");
            std::string_view sv = *s;

            REQUIRE(sv == "hello");
            REQUIRE(sv.size() == 5);
        }

        TEST_CASE("DString-cstr-conversion", "[object2][DString]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DString * s = DString::from_cstr(alloc, "hello");
            const char * cstr = *s;

            REQUIRE(std::strcmp(cstr, "hello") == 0);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end DString.test.cpp */
