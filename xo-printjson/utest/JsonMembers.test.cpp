/** @file JsonMembers.test.cpp
 *
 *  @author Roland Conybeare, Oct 2026
 *
 *  xo::json::JsonMembers: the "_members_" array a JsonPrinter writes for the
 *  C++ members it opts in to showing.  See .xo-backlog/xo-websock/issues/13.
 **/

#include "xo/printjson/JsonMembers.hpp"
#include "xo/printjson/PrintJson.hpp"
#include "xo/printjson/init_printjson.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/StructReflector.hpp>
#include <xo/reflectutil/type_name.hpp>
#include <catch2/catch.hpp>
#include <atomic>
#include <sstream>
#include <string>
#include <vector>

namespace xo {
    using xo::json::JsonMembers;
    using xo::json::PrintJson;
    using xo::reflect::StructReflector;

    namespace ut {
        namespace {
            InitEvidence s_init_evidence = InitSubsys<S_printjson_tag>::require();

            /* a reflected struct, members a_ and b_ */
            struct JmReflected { int a_ = 1; std::string b_ = "x"; };

            /* never reflected */
            struct JmUnreflected { int z_ = 0; };

            void reflect_jm_types() {
                StructReflector<JmReflected> sr;

                if (sr.is_incomplete()) {
                    REFLECT_MEMBER(sr, a);
                    REFLECT_MEMBER(sr, b);
                }
            }

            /** the "_type_" text for T: its canonical name **/
            template <typename T>
            std::string type_of() {
                return std::string(xo::reflect::type_name<T>());
            }

            /** one entry, as JsonMembers writes it **/
            std::string entry(std::string const & name, std::string const & type,
                              std::string const & value) {
                return "{\"_name_\": \"" + name + "\", \"_type_\": \"" + type
                    + "\", \"_value_\": " + value + "}";
            }
        }

        TEST_CASE("json-members-empty", "[printjson][JsonMembers]") {
            PrintJson pjson;
            std::stringstream ss;

            JsonMembers mem(&pjson, &ss);
            mem.end();

            REQUIRE(ss.str() == ", \"_members_\": []");
        }

        TEST_CASE("json-members-scalars-and-strings", "[printjson][JsonMembers]") {
            PrintJson pjson;
            std::stringstream ss;

            int n = 7;
            std::string s = "hello";

            JsonMembers mem(&pjson, &ss);
            mem.member("n_", n).member("s_", s);
            mem.end();

            REQUIRE(ss.str() == ", \"_members_\": ["
                    + entry("n_", type_of<int>(), "7") + ", "
                    + entry("s_", type_of<std::string>(), "\"hello\"") + "]");
        }

        TEST_CASE("json-members-reflected-struct", "[printjson][JsonMembers]") {
            reflect_jm_types();

            PrintJson pjson;
            std::stringstream ss;

            JmReflected r;
            JmReflected * p = &r;
            std::vector<JmReflected> v(1);

            JsonMembers mem(&pjson, &ss);
            mem.member("r_", r).member("p_", p).member("v_", v);
            mem.end();

            /* the value printed as PrintJson prints any value */
            std::stringstream rs;
            pjson.print(r, &rs);

            REQUIRE(ss.str() == ", \"_members_\": ["
                    + entry("r_", type_of<JmReflected>(), rs.str()) + ", "
                    + entry("p_", type_of<JmReflected *>(), rs.str()) + ", "
                    + entry("v_", type_of<std::vector<JmReflected>>(), "[" + rs.str() + "]")
                    + "]");
        }

        TEST_CASE("json-members-unreflected-type-is-an-error-entry", "[printjson][JsonMembers]") {
            PrintJson pjson;
            std::stringstream ss;

            JmUnreflected u;
            JmUnreflected * pu = &u;

            JsonMembers mem(&pjson, &ss);
            mem.member("u_", u).member("pu_", pu).member("n_", 3);
            mem.end();

            /* the entry says why, carries no value, and the rest still prints */
            std::string const why = "type not reflected: " + type_of<JmUnreflected>();

            REQUIRE(ss.str() == ", \"_members_\": ["
                    "{\"_name_\": \"u_\", \"_type_\": \"" + type_of<JmUnreflected>()
                    + "\", \"_error_\": \"" + why + "\"}, "
                    "{\"_name_\": \"pu_\", \"_type_\": \"" + type_of<JmUnreflected *>()
                    + "\", \"_error_\": \"" + why + "\"}, "
                    + entry("n_", type_of<int>(), "3") + "]");
        }

        TEST_CASE("json-members-member-as-declared-type", "[printjson][JsonMembers]") {
            /* a member read through load(): _type_ is the declared type, the
             * value whatever the printer read
             */
            PrintJson pjson;
            std::stringstream ss;

            std::atomic<std::int32_t> port{8080};

            JsonMembers mem(&pjson, &ss);
            mem.member_as<std::atomic<std::int32_t>>("port_", port.load());
            mem.end();

            REQUIRE(ss.str() == ", \"_members_\": ["
                    + entry("port_", type_of<std::atomic<std::int32_t>>(), "8080") + "]");
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end JsonMembers.test.cpp */
