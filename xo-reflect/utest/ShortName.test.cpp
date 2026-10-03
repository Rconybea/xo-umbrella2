/** @file ShortName.test.cpp
 *
 *  TypeDescrBase::make_short_name(): a type's canonical name for display,
 *  without excess detail -- qualifiers, rp<> for intrusive_ptr<>, default
 *  template arguments.
 **/

#include "xo/reflect/Reflect.hpp"
#include "xo/reflect/TypeDescr.hpp"
#include <xo/reflectutil/type_name.hpp>
#include <catch2/catch.hpp>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TypeDescrBase;

    namespace ut {
        namespace {
            struct ShortNameProbe : public ref::Refcount {};

            std::string short_of(std::string_view canonical) {
                return TypeDescrBase::make_short_name(canonical);
            }
        }

        TEST_CASE("short-name-spellings", "[reflect][short_name]") {
            /* plain names */
            REQUIRE(short_of("int") == "int");
            REQUIRE(short_of("long unsigned int") == "long unsigned int");
            REQUIRE(short_of("xo::option::Px2") == "Px2");

            /* template arguments lose their qualifiers too */
            REQUIRE(short_of("xo::web::WsSessionTable<xo::web::WebsocketSessionRecd>")
                    == "WsSessionTable<WebsocketSessionRecd>");
            REQUIRE(short_of("std::pair<int, double>") == "pair<int, double>");

            /* xo's intrusive_ptr is rp; anyone else's is not */
            REQUIRE(short_of("xo::ref::intrusive_ptr<xo::web::DynamicEndpoint>")
                    == "rp<DynamicEndpoint>");
            REQUIRE(short_of("std::vector<xo::ref::intrusive_ptr<xo::web::Foo> >")
                    == "vector<rp<Foo>>");
            REQUIRE(short_of("boost::intrusive_ptr<a::Foo>") == "intrusive_ptr<Foo>");

            /* anonymous namespaces, as gcc and clang spell them */
            REQUIRE(short_of("xo::web::{anonymous}::Subscription") == "Subscription");
            REQUIRE(short_of("xo::web::(anonymous namespace)::Subscription") == "Subscription");

            /* default template arguments go, explicit others stay */
            REQUIRE(short_of("std::unordered_map<std::__cxx11::basic_string<char>, "
                             "xo::ref::intrusive_ptr<xo::web::DynamicEndpoint>, "
                             "std::hash<std::__cxx11::basic_string<char> >, "
                             "std::equal_to<std::__cxx11::basic_string<char> >, "
                             "std::allocator<std::pair<const std::__cxx11::basic_string<char>, "
                             "xo::ref::intrusive_ptr<xo::web::DynamicEndpoint> > > >")
                    == "unordered_map<string, rp<DynamicEndpoint>>");
            REQUIRE(short_of("std::unique_ptr<a::Foo, std::default_delete<a::Foo> >") == "unique_ptr<Foo>");
            REQUIRE(short_of("std::map<int, a::Foo, std::greater<int> >") == "map<int, Foo, greater<int>>");
            REQUIRE(short_of("std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >")
                    == "string");

            /* a member pointer keeps its class */
            REQUIRE(short_of("int a::Foo::*") == "int Foo::*");
        }

        TEST_CASE("short-name-reflected", "[reflect][short_name]") {
            /* as this compiler spells them */
            REQUIRE(Reflect::require<ShortNameProbe>()->short_name() == "ShortNameProbe");
            REQUIRE(Reflect::require<rp<ShortNameProbe>>()->short_name() == "rp<ShortNameProbe>");
            REQUIRE(Reflect::require<std::vector<std::string>>()->short_name() == "vector<string>");
            REQUIRE(Reflect::require<std::pair<int, double>>()->short_name() == "pair<int, double>");

            /* canonical name unchanged.  Built, not spelled out: the anonymous
             * namespace is "{anonymous}" under gcc, "(anonymous namespace)"
             * under clang
             */
            REQUIRE(Reflect::require<ShortNameProbe>()->canonical_name()
                    == std::string(xo::reflect::type_name<ShortNameProbe>()));
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end ShortName.test.cpp */
