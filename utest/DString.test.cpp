/** @file DString.test.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include <xo/object2/StringOps.hpp>
#include <xo/alloc2/arena/IAllocator_DArena.hpp>
#include <catch2/catch.hpp>
#include <cctype>
#include <cstring>
#include <string>

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

        TEST_CASE("DString-from_view", "[object2][DString]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            std::string_view sv = "hello world";
            DString * s = DString::from_view(alloc, sv);

            REQUIRE(s != nullptr);
            REQUIRE(s->capacity() == 12);
            REQUIRE(s->size() == 11);
            REQUIRE(std::strcmp(s->chars(), "hello world") == 0);

            // test with substring (not null-terminated)
            std::string_view sub = sv.substr(0, 5);
            DString * s2 = DString::from_view(alloc, sub);

            REQUIRE(s2 != nullptr);
            REQUIRE(s2->capacity() == 6);
            REQUIRE(s2->size() == 5);
            REQUIRE(std::strcmp(s2->chars(), "hello") == 0);
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

        TEST_CASE("DString-begin-end", "[object2][DString]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DString * s = DString::from_cstr(alloc, "hello");

            REQUIRE(s->begin() == s->chars());
            REQUIRE(s->end() == s->chars() + 5);
            REQUIRE(s->end() - s->begin() == 5);

            *s->begin() = 'H';
            REQUIRE(std::strcmp(s->chars(), "Hello") == 0);
        }

        TEST_CASE("DString-cbegin-cend", "[object2][DString]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DString * s = DString::from_cstr(alloc, "hello");

            REQUIRE(s->cbegin() == s->chars());
            REQUIRE(s->cend() == s->chars() + 5);
            REQUIRE(s->cend() - s->cbegin() == 5);
        }

        TEST_CASE("DString-range-for", "[object2][DString]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DString * s = DString::from_cstr(alloc, "hello");

            std::string result;
            for (char c : *s) {
                result += c;
            }

            REQUIRE(result == "hello");
        }

        TEST_CASE("DString-iterator-modify", "[object2][DString]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DString * s = DString::from_cstr(alloc, "hello");

            for (auto it = s->begin(); it != s->end(); ++it) {
                *it = std::toupper(*it);
            }

            REQUIRE(std::strcmp(s->chars(), "HELLO") == 0);
        }

        TEST_CASE("DString-clone", "[object2][DString]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DString * src = DString::from_cstr(alloc, "hello world");
            DString * copy = DString::clone(alloc, src);

            REQUIRE(copy != nullptr);
            REQUIRE(copy != src);
            REQUIRE(copy->size() == src->size());
            REQUIRE(copy->capacity() == src->capacity());
            REQUIRE(std::strcmp(copy->chars(), src->chars()) == 0);
        }

        TEST_CASE("DString-sprintf", "[object2][DString]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DString * s = DString::empty(alloc, 32);

            auto n = s->sprintf("hello %s %d", "world", 42);

            REQUIRE(n == 14);
            REQUIRE(s->size() == 14);
            REQUIRE(std::strcmp(s->chars(), "hello world 42") == 0);
        }

        TEST_CASE("DString-sprintf-truncate", "[object2][DString]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DString * s = DString::empty(alloc, 8);

            auto n = s->sprintf("hello world");

            REQUIRE(n == 7);
            REQUIRE(s->size() == 7);
            REQUIRE(std::strcmp(s->chars(), "hello w") == 0);
        }

        TEST_CASE("DString-compare", "[object2][DString]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DString * s1 = DString::from_cstr(alloc, "apple");
            DString * s2 = DString::from_cstr(alloc, "apple");
            DString * s3 = DString::from_cstr(alloc, "banana");
            DString * s4 = DString::from_cstr(alloc, "aardvark");

            REQUIRE(DString::compare(*s1, *s2) == 0);
            REQUIRE(DString::compare(*s1, *s3) < 0);
            REQUIRE(DString::compare(*s3, *s1) > 0);
            REQUIRE(DString::compare(*s1, *s4) > 0);
            REQUIRE(DString::compare(*s4, *s1) < 0);
        }

        TEST_CASE("DString-comparison-operators", "[object2][DString]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DString * apple1 = DString::from_cstr(alloc, "apple");
            DString * apple2 = DString::from_cstr(alloc, "apple");
            DString * banana = DString::from_cstr(alloc, "banana");

            // operator==
            REQUIRE(*apple1 == *apple1);
            REQUIRE(*apple2 == *apple2);
            REQUIRE(*banana == *banana);
            REQUIRE(*apple1 == *apple2);

            REQUIRE_FALSE(*apple1 == *banana);

            // operator!=
            REQUIRE(*apple1 != *banana);
            REQUIRE_FALSE(*apple1 != *apple1);
            REQUIRE_FALSE(*apple2 != *apple2);
            REQUIRE_FALSE(*apple1 != *apple2);
            REQUIRE_FALSE(*banana != *banana);

            // operator<
            REQUIRE(*apple1 < *banana);
            REQUIRE_FALSE(*banana < *apple1);
            REQUIRE_FALSE(*apple1 < *apple2);

            // operator<=
            REQUIRE(*apple1 <= *banana);
            REQUIRE(*apple1 <= *apple2);
            REQUIRE_FALSE(*banana <= *apple1);

            // operator>
            REQUIRE(*banana > *apple1);
            REQUIRE_FALSE(*apple1 > *banana);
            REQUIRE_FALSE(*apple1 > *apple2);

            // operator>=
            REQUIRE(*banana >= *apple1);
            REQUIRE(*apple1 >= *apple2);
            REQUIRE_FALSE(*apple1 >= *banana);
        }

        TEST_CASE("DString-hash", "[object2][DString]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DString * s1 = DString::from_cstr(alloc, "hello");
            DString * s2 = DString::from_cstr(alloc, "hello");
            DString * s3 = DString::from_cstr(alloc, "world");
            DString * empty1 = DString::empty(alloc, 16);
            DString * empty2 = DString::empty(alloc, 32);

            // same content produces same hash
            REQUIRE(s1->hash() == s2->hash());

            // empty strings have same hash
            REQUIRE(empty1->hash() == empty2->hash());

            // different content produces different hash (not guaranteed, but highly likely)
            REQUIRE(s1->hash() != s3->hash());

            // std::hash specialization works
            std::hash<DString> hasher;
            REQUIRE(hasher(*s1) == s1->hash());
            REQUIRE(hasher(*s2) == s2->hash());
            REQUIRE(hasher(*s1) == hasher(*s2));
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end DString.test.cpp */
