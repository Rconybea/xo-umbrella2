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
    using xo::json::JsonPrintState;
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

            /* members of a reflected struct, for reflected_members(): a
             * value, a pointer to it, a vector of them; literal names
             */
            struct JmHolder {
                JmReflected r_;
                JmReflected * p_ = nullptr;
                std::vector<JmReflected> v_;
            };

            /* a reflected struct with a member whose type is not */
            struct JmBad {
                JmUnreflected u_;
            };

            void reflect_jm_holder() {
                reflect_jm_types();

                {
                    StructReflector<JmHolder> sr;

                    if (sr.is_incomplete()) {
                        REFLECT_LITERAL_MEMBER(sr, r_);
                        REFLECT_LITERAL_MEMBER(sr, p_);
                        REFLECT_LITERAL_MEMBER(sr, v_);
                    }
                }
                {
                    StructReflector<JmBad> sr;

                    if (sr.is_incomplete())
                        REFLECT_LITERAL_MEMBER(sr, u_);
                }
            }

            /** the "_canonical_type_" text for T: its canonical name **/
            template <typename T>
            std::string type_of() {
                return std::string(xo::reflect::type_name<T>());
            }

            /** one entry, as JsonMembers writes it; @p metatype is the
             *  declared type's, as xo-reflect's metatype2str spells it
             **/
            std::string entry(std::string const & name, std::string const & type,
                              std::string const & metatype, std::string const & value) {
                return "{\"_name_\": \"" + name + "\", \"_canonical_type_\": \"" + type
                    + "\", \"_short_type_\": \"" + xo::reflect::TypeDescrBase::make_short_name(type)
                    + "\", \"_metatype_\": \"" + metatype
                    + "\", \"_value_\": " + value + "}";
            }
        }

        TEST_CASE("json-members-empty", "[printjson][JsonMembers]") {
            PrintJson pjson;
            std::stringstream ss;

            JsonPrintState state(&pjson, &ss);

            JsonMembers mem(state);
            mem.end();

            REQUIRE(ss.str() == ", \"_members_\": []");
        }

        TEST_CASE("json-members-scalars-and-strings", "[printjson][JsonMembers]") {
            PrintJson pjson;
            std::stringstream ss;

            int n = 7;
            std::string s = "hello";

            JsonPrintState state(&pjson, &ss);

            JsonMembers mem(state);
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

            JsonPrintState state(&pjson, &ss);

            JsonMembers mem(state);
            mem.member("r_", r).member("p_", p).member("v_", v);
            mem.end();

            /* the value printed as PrintJson prints any value: r_ in full,
             * object 1; p_ points at r, printed already -- a ref; v_'s
             * element is another object, 2
             */
            auto jm = [](int id) {
                return "{\"_name_\": \"JmReflected\", \"_canonical_type_\": \"" + type_of<JmReflected>()
                    + "\", \"_short_type_\": \"JmReflected\", \"_id_\": " + std::to_string(id)
                    + ", \"_members_\": ["
                    + entry("a", type_of<int>(), "atomic", "1") + ", "
                    + entry("b", type_of<std::string>(), "atomic", "\"x\"") + "]}";
            };

            REQUIRE(ss.str() == ", \"_members_\": ["
                    + entry("r_", type_of<JmReflected>(), "struct", jm(1)) + ", "
                    + entry("p_", type_of<JmReflected *>(), "pointer", "{\"_ref_\": 1}") + ", "
                    + entry("v_", type_of<std::vector<JmReflected>>(), "vector", "[" + jm(2) + "]")
                    + "]");
        }

        TEST_CASE("json-members-unreflected-type-is-an-error-entry", "[printjson][JsonMembers]") {
            PrintJson pjson;
            std::stringstream ss;

            JmUnreflected u;
            JmUnreflected * pu = &u;

            JsonPrintState state(&pjson, &ss);

            JsonMembers mem(state);
            mem.member("u_", u).member("pu_", pu).member("n_", 3);
            mem.end();

            /* the entry says why, carries no value, and the rest still prints */
            std::string const why = "type not reflected: " + type_of<JmUnreflected>();

            REQUIRE(ss.str() == ", \"_members_\": ["
                    "{\"_name_\": \"u_\", \"_canonical_type_\": \"" + type_of<JmUnreflected>()
                    + "\", \"_short_type_\": \"" + xo::reflect::TypeDescrBase::make_short_name(type_of<JmUnreflected>())
                    + "\", \"_metatype_\": \"atomic\", \"_error_\": \"" + why + "\"}, "
                    "{\"_name_\": \"pu_\", \"_canonical_type_\": \"" + type_of<JmUnreflected *>()
                    + "\", \"_short_type_\": \"" + xo::reflect::TypeDescrBase::make_short_name(type_of<JmUnreflected *>())
                    + "\", \"_metatype_\": \"pointer\", \"_error_\": \"" + why + "\"}, "
                    + entry("n_", type_of<int>(), "atomic", "3") + "]");
        }

        TEST_CASE("json-members-member-as-declared-type", "[printjson][JsonMembers]") {
            /* a member read through load(): the type keys name the declared type, the
             * value whatever the printer read
             */
            PrintJson pjson;
            std::stringstream ss;

            std::atomic<std::int32_t> port{8080};

            JsonPrintState state(&pjson, &ss);

            JsonMembers mem(state);
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

            JsonPrintState state(&pjson, &ss);

            JsonMembers mem(state);
            mem.member_ref<JmReflected *>("r_", &r)
                .member_ref<JmReflected *>("null_", null_p);
            mem.end();

            REQUIRE(ss.str() == ", \"_members_\": ["
                    + entry("r_", type_of<JmReflected *>(), "pointer",
                            "{\"_ref_\": 1}") + ", "
                    + entry("null_", type_of<JmReflected *>(), "pointer", "null") + "]");
        }

        TEST_CASE("json-members-member-ref-to-a-reference", "[printjson][JsonMembers]") {
            /* a C++ reference member: reflection has no metatype for
             * references -- reported as pointer, the nearest
             */
            PrintJson pjson;
            std::stringstream ss;

            JmReflected r;
            JmReflected const & cr = r;

            JsonPrintState state(&pjson, &ss);

            JsonMembers mem(state);
            mem.member_ref<JmReflected const &>("cr_", &cr);
            mem.end();

            REQUIRE(ss.str() == ", \"_members_\": ["
                    + entry("cr_", type_of<JmReflected const &>(), "pointer",
                            "{\"_ref_\": 1}") + "]");
        }

        TEST_CASE("json-members-member-refs", "[printjson][JsonMembers]") {
            /* a container of objects printed elsewhere: an array of refs, a
             * released slot null -- slot positions kept
             */
            PrintJson pjson;
            std::stringstream ss;

            JmReflected a, b;

            JsonPrintState state(&pjson, &ss);

            JsonMembers mem(state);
            mem.member_refs<std::vector<JmReflected *>>("v_", {&a, nullptr, &b});
            mem.end();

            REQUIRE(ss.str() == ", \"_members_\": ["
                    + entry("v_", type_of<std::vector<JmReflected *>>(), "vector",
                            "[{\"_ref_\": 1}, null, {\"_ref_\": 2}]") + "]");
        }

        TEST_CASE("json-members-member-ref-map", "[printjson][JsonMembers]") {
            /* a map to objects printed elsewhere: a json object, key -> ref
             * (or null), keys in the order given
             */
            PrintJson pjson;
            std::stringstream ss;

            JmReflected a, b;

            JsonPrintState state(&pjson, &ss);

            JsonMembers mem(state);
            mem.member_ref_map<std::map<std::string, JmReflected *>>("m_", {{"/a/", &a}, {"/b/", &b}, {"/z/", nullptr}});
            mem.end();

            REQUIRE(ss.str() == ", \"_members_\": ["
                    + entry("m_", type_of<std::map<std::string, JmReflected *>>(), "atomic",
                            "{\"/a/\": {\"_ref_\": 1}, "
                            "\"/b/\": {\"_ref_\": 2}, "
                            "\"/z/\": null}") + "]");
        }
        TEST_CASE("json-members-reflected-members-as-member-would", "[printjson][JsonMembers]") {
            /* each reflected member, in reflection order, exactly as
             * member() writes it -- a ref and ids included: p_ points at
             * r_, printed already
             */
            reflect_jm_holder();

            PrintJson pjson;

            JmHolder h;
            h.p_ = &h.r_;
            h.v_.resize(1);

            std::stringstream by_hand;
            {
                JsonPrintState state(&pjson, &by_hand);
                JsonMembers mem(state);
                mem.member("r_", h.r_).member("p_", h.p_).member("v_", h.v_);
                mem.end();
            }

            std::stringstream reflected;
            {
                JsonPrintState state(&pjson, &reflected);
                JsonMembers mem(state);
                mem.reflected_members(xo::reflect::Reflect::make_tp(&h));
                mem.end();
            }

            INFO("by hand:   " << by_hand.str());
            INFO("reflected: " << reflected.str());
            REQUIRE(reflected.str() == by_hand.str());
            REQUIRE(reflected.str().find("{\"_ref_\": 1}") != std::string::npos);
        }

        TEST_CASE("json-members-reflected-member-not-printable", "[printjson][JsonMembers]") {
            /* a reflected member whose type cannot print: an error entry,
             * as member() writes one
             */
            reflect_jm_holder();

            PrintJson pjson;
            std::stringstream ss;

            JmBad b;

            JsonPrintState state(&pjson, &ss);
            JsonMembers mem(state);
            mem.reflected_members(xo::reflect::Reflect::make_tp(&b));
            mem.end();

            REQUIRE(ss.str().find("\"_name_\": \"u_\"") != std::string::npos);
            REQUIRE(ss.str().find("\"_error_\": \"type not reflected: ") != std::string::npos);
        }

        TEST_CASE("json-members-reflected-members-name-suffix", "[printjson][JsonMembers]") {
            /* JmReflected reflects a_ as "a" (REFLECT_MEMBER): the suffix
             * restores the C++ name
             */
            reflect_jm_types();

            PrintJson pjson;

            JmReflected r;

            std::stringstream plain, suffixed;
            {
                JsonPrintState state(&pjson, &plain);
                JsonMembers mem(state);
                mem.reflected_members(xo::reflect::Reflect::make_tp(&r));
                mem.end();
            }
            {
                JsonPrintState state(&pjson, &suffixed);
                JsonMembers mem(state);
                mem.reflected_members(xo::reflect::Reflect::make_tp(&r), "_");
                mem.end();
            }

            REQUIRE(plain.str() == ", \"_members_\": ["
                    + entry("a", type_of<int>(), "atomic", "1") + ", "
                    + entry("b", type_of<std::string>(), "atomic", "\"x\"") + "]");
            REQUIRE(suffixed.str() == ", \"_members_\": ["
                    + entry("a_", type_of<int>(), "atomic", "1") + ", "
                    + entry("b_", type_of<std::string>(), "atomic", "\"x\"") + "]");
        }

        TEST_CASE("json-members-reflected-members-of-a-non-struct", "[printjson][JsonMembers]") {
            PrintJson pjson;
            std::stringstream ss;

            int x = 7;

            JsonPrintState state(&pjson, &ss);
            JsonMembers mem(state);
            mem.reflected_members(xo::reflect::Reflect::make_tp(&x));
            mem.end();

            REQUIRE(ss.str() == ", \"_members_\": []");
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end JsonMembers.test.cpp */
