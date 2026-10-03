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
#include <map>
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

            /** one entry, as JsonMembers writes it; @p metatype is the
             *  declared type's, as xo-reflect's metatype2str spells it
             **/
            std::string entry(std::string const & name, std::string const & type,
                              std::string const & metatype, std::string const & value) {
                return "{\"_name_\": \"" + name + "\", \"_type_\": \"" + type
                    + "\", \"_metatype_\": \"" + metatype
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
                    + entry("n_", type_of<int>(), "atomic", "7") + ", "
                    + entry("s_", type_of<std::string>(), "atomic", "\"hello\"") + "]");
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
                    + entry("r_", type_of<JmReflected>(), "struct", rs.str()) + ", "
                    + entry("p_", type_of<JmReflected *>(), "pointer", rs.str()) + ", "
                    + entry("v_", type_of<std::vector<JmReflected>>(), "vector", "[" + rs.str() + "]")
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
                    + "\", \"_metatype_\": \"atomic\", \"_error_\": \"" + why + "\"}, "
                    "{\"_name_\": \"pu_\", \"_type_\": \"" + type_of<JmUnreflected *>()
                    + "\", \"_metatype_\": \"pointer\", \"_error_\": \"" + why + "\"}, "
                    + entry("n_", type_of<int>(), "atomic", "3") + "]");
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
                    + entry("port_", type_of<std::atomic<std::int32_t>>(), "atomic", "8080") + "]");
        }

        TEST_CASE("json-members-member-ref", "[printjson][JsonMembers]") {
            /* an object printed elsewhere: its id, so a consumer can join the
             * two -- the same id that object's own printer writes
             */
            PrintJson pjson;
            std::stringstream ss;

            JmReflected r;
            JmReflected * null_p = nullptr;

            JsonMembers mem(&pjson, &ss);
            mem.member_ref<JmReflected *>("r_", &r)
                .member_ref<JmReflected *>("null_", null_p);
            mem.end();

            REQUIRE(ss.str() == ", \"_members_\": ["
                    + entry("r_", type_of<JmReflected *>(), "pointer",
                            "{\"ref\": \"" + xo::json::json_id(&r) + "\"}") + ", "
                    + entry("null_", type_of<JmReflected *>(), "pointer", "null") + "]");

            /* the id is the address, as written by ostream */
            std::stringstream addr;
            addr << static_cast<void const *>(&r);
            REQUIRE(xo::json::json_id(&r) == addr.str());
        }

        TEST_CASE("json-members-member-ref-to-a-reference", "[printjson][JsonMembers]") {
            /* a C++ reference member: reflection has no metatype for
             * references -- reported as pointer, the nearest
             */
            PrintJson pjson;
            std::stringstream ss;

            JmReflected r;
            JmReflected const & cr = r;

            JsonMembers mem(&pjson, &ss);
            mem.member_ref<JmReflected const &>("cr_", &cr);
            mem.end();

            REQUIRE(ss.str() == ", \"_members_\": ["
                    + entry("cr_", type_of<JmReflected const &>(), "pointer",
                            "{\"ref\": \"" + xo::json::json_id(&r) + "\"}") + "]");
        }

        TEST_CASE("json-members-member-refs", "[printjson][JsonMembers]") {
            /* a container of objects printed elsewhere: an array of refs, a
             * released slot null -- slot positions kept
             */
            PrintJson pjson;
            std::stringstream ss;

            JmReflected a, b;

            JsonMembers mem(&pjson, &ss);
            mem.member_refs<std::vector<JmReflected *>>("v_", {&a, nullptr, &b});
            mem.end();

            REQUIRE(ss.str() == ", \"_members_\": ["
                    + entry("v_", type_of<std::vector<JmReflected *>>(), "vector",
                            "[{\"ref\": \"" + xo::json::json_id(&a) + "\"}, null, "
                            "{\"ref\": \"" + xo::json::json_id(&b) + "\"}]") + "]");
        }

        TEST_CASE("json-members-member-ref-map", "[printjson][JsonMembers]") {
            /* a map to objects printed elsewhere: a json object, key -> ref
             * (or null), keys in the order given
             */
            PrintJson pjson;
            std::stringstream ss;

            JmReflected a, b;

            JsonMembers mem(&pjson, &ss);
            mem.member_ref_map<std::map<std::string, JmReflected *>>("m_", {{"/a/", &a}, {"/b/", &b}, {"/z/", nullptr}});
            mem.end();

            REQUIRE(ss.str() == ", \"_members_\": ["
                    + entry("m_", type_of<std::map<std::string, JmReflected *>>(), "atomic",
                            "{\"/a/\": {\"ref\": \"" + xo::json::json_id(&a) + "\"}, "
                            "\"/b/\": {\"ref\": \"" + xo::json::json_id(&b) + "\"}, "
                            "\"/z/\": null}") + "]");
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end JsonMembers.test.cpp */
