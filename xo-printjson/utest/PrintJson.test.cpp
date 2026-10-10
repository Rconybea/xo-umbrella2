/* file PrintJson.test.cpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#include "xo/printjson/PrintJson.hpp"
#include "xo/printjson/init_printjson.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/StructReflector.hpp>
#include <xo/reflect/EnumReflector.hpp>
#include <xo/reflect/WrapperReflector.hpp>
#include <xo/reflectutil/type_name.hpp>
#include <xo/ppsink/tag_ostream.hpp>   /* os << tag(..) */
#include <xo/arena/DArenaVector.hpp>
#include <catch2/catch.hpp>
#include <atomic>
#include <iostream>
#include <memory>
#include <sstream>

//#define STRINGIFY(x) #x

namespace xo {
    using xo::json::PrintJson;
    using xo::reflect::Reflect;
    using xo::reflect::StructReflector;
    using xo::reflect::TaggedPtr;

    namespace ut {
        /* one scope in from namespace xo: a using-decl at xo scope would be
         * ambiguous with legacy xo::tag rather than shadowing it.
         */
        using xo::pp::tag;

        InitEvidence s_init_evidence = InitSubsys<S_printjson_tag>::require();

        /** the type keys the struct printer emits for T, after "_name_":
         *  its canonical and short names.  Built, not spelled out: a type in
         *  an anonymous namespace is "{anonymous}" under gcc, "(anonymous
         *  namespace)" under clang
         **/
        template <typename T>
        std::string type_member() {
            std::string canonical(xo::reflect::type_name<T>());
            return ", \"_canonical_type_\": \"" + canonical + "\""
                + ", \"_short_type_\": \""
                + xo::reflect::TypeDescrBase::make_short_name(canonical) + "\"";
        }

        /** one "_members_" entry, as the struct printer writes it: member
         *  @p name, declared type T (metatype @p metatype), printed as
         *  @p value
         **/
        template <typename T>
        std::string mentry(char const * name, char const * metatype, std::string const & value) {
            std::string canonical(xo::reflect::type_name<T>());
            return "{\"_name_\": \"" + std::string(name) + "\""
                + ", \"_canonical_type_\": \"" + canonical + "\""
                + ", \"_short_type_\": \""
                + xo::reflect::TypeDescrBase::make_short_name(canonical) + "\""
                + ", \"_metatype_\": \"" + metatype + "\""
                + ", \"_value_\": " + value + "}";
        }

        namespace {
            struct TestStruct0 {};

            /** arena-backed vector, for the DArenaVector cases below **/
            template <typename T>
            xo::mm::DArenaVector<T> make_arena_vec(const char * name) {
                using xo::mm::ArenaConfig;
                using xo::mm::ArenaNameStr;

                return xo::mm::DArenaVector<T>::map(ArenaConfig()
                                                    .with_name(ArenaNameStr::from_cstr(name))
                                                    .with_size(64*1024));
            }
        }

        namespace {
            struct DPtrTarget {
                double v_;
            };

            /** a struct with a RAW POINTER member -- the shape
             *  .xo-backlog/xo-reflect/issues/01 exists for.  FlywheelInfo
             *  ::strong_ is the real instance of it.
             **/
            struct DPtrHolder {
                const DPtrTarget * p_;
            };

            void reflect_ptr_types() {
                static bool s_once = []() {
                    {
                        StructReflector<DPtrTarget> sr;
                        sr.reflect_member("v", &DPtrTarget::v_);
                        sr.require_complete();
                    }
                    {
                        StructReflector<DPtrHolder> sr;
                        /* shared: a raw pointer is borrowed by default,
                         * printing only a ref (.xo-backlog/xo-printjson/issues/08);
                         * this case is about rendering the pointee
                         */
                        sr.reflect_member("p", &DPtrHolder::p_).shared();
                        sr.require_complete();
                    }
                    return true;
                }();
                (void)s_once;
            }
        }

        TEST_CASE("print-json-raw-pointer-member", "[printjson][rawpointer]") {
            /* the member renders as its POINTEE, not as an address and not as
             * an opaque atom.  Before raw pointers were reflected this printed
             * <error-json-printer-not-found ... mt_atomic>.
             */
            reflect_ptr_types();

            PrintJson print_json;

            DPtrTarget target{1.5};
            DPtrHolder holder{&target};

            std::stringstream ss;
            print_json.print(holder, &ss);

            std::string const target_json
                = ("{\"_name_\": \"DPtrTarget\"" + type_member<DPtrTarget>()
                   + ", \"_id_\": 2, \"_members_\": [" + mentry<double>("v", "atomic", "1.5") + "]}");

            REQUIRE(ss.str() == ("{\"_name_\": \"DPtrHolder\"" + type_member<DPtrHolder>()
                                 + ", \"_id_\": 1, \"_members_\": ["
                                 + mentry<const DPtrTarget *>("p", "pointer", target_json) + "]}"));
        } /*TEST_CASE(print-json-raw-pointer-member)*/

        TEST_CASE("print-json-null-raw-pointer-member", "[printjson][rawpointer]") {
            /* json null, not "{}".  print_generic_pointer emitted "{}" until
             * 2026-09-21, distinguishable from a real struct only by the
             * absent _name_ member.
             */
            reflect_ptr_types();

            PrintJson print_json;

            DPtrHolder holder{nullptr};

            std::stringstream ss;
            print_json.print(holder, &ss);

            REQUIRE(ss.str() == ("{\"_name_\": \"DPtrHolder\"" + type_member<DPtrHolder>()
                                 + ", \"_id_\": 1, \"_members_\": ["
                                 + mentry<const DPtrTarget *>("p", "pointer", "null") + "]}"));
        } /*TEST_CASE(print-json-null-raw-pointer-member)*/

        namespace {
            /** a struct with a std::unique_ptr member (.xo-backlog/xo-reflect/issues/04) **/
            struct UPtrHolder {
                std::unique_ptr<DPtrTarget> p_;
            };

            void reflect_uptr_types() {
                reflect_ptr_types();

                StructReflector<UPtrHolder> sr;

                if (sr.is_incomplete())
                    REFLECT_MEMBER(sr, p);
            }
        }

        TEST_CASE("print-json-unique-ptr-member", "[printjson][uniqueptr]") {
            /* as a raw pointer member prints: its pointee, in full */
            reflect_uptr_types();

            PrintJson print_json;

            UPtrHolder holder{std::make_unique<DPtrTarget>(DPtrTarget{2.5})};

            std::stringstream ss;
            print_json.print(holder, &ss);

            std::string const target_json
                = ("{\"_name_\": \"DPtrTarget\"" + type_member<DPtrTarget>()
                   + ", \"_id_\": 2, \"_members_\": [" + mentry<double>("v", "atomic", "2.5") + "]}");

            REQUIRE(ss.str() == ("{\"_name_\": \"UPtrHolder\"" + type_member<UPtrHolder>()
                                 + ", \"_id_\": 1, \"_members_\": ["
                                 + mentry<std::unique_ptr<DPtrTarget>>("p", "pointer", target_json) + "]}"));
        } /*TEST_CASE(print-json-unique-ptr-member)*/

        TEST_CASE("print-json-null-unique-ptr-member", "[printjson][uniqueptr]") {
            reflect_uptr_types();

            PrintJson print_json;

            UPtrHolder holder;

            std::stringstream ss;
            print_json.print(holder, &ss);

            REQUIRE(ss.str() == ("{\"_name_\": \"UPtrHolder\"" + type_member<UPtrHolder>()
                                 + ", \"_id_\": 1, \"_members_\": ["
                                 + mentry<std::unique_ptr<DPtrTarget>>("p", "pointer", "null") + "]}"));
        } /*TEST_CASE(print-json-null-unique-ptr-member)*/

        TEST_CASE("print-json-null-c-string", "[printjson][rawpointer]") {
            /* REGRESSION: quot(nullptr) segfaulted.  char* and const char*
             * have been registered with provide_string_printer since long
             * before raw-pointer reflection, so this was reachable already --
             * it just had no test.  Measured as a crash 2026-09-21.
             */
            PrintJson print_json;

            const char * s = nullptr;

            std::stringstream ss;
            print_json.print(s, &ss);

            REQUIRE(ss.str() == std::string("null"));

            /* a non-null one still renders as a quoted string, i.e. the
             * exemption from T* held
             */
            const char * t = "abc";

            std::stringstream ss2;
            print_json.print(t, &ss2);

            REQUIRE(ss2.str() == std::string("\"abc\""));
        } /*TEST_CASE(print-json-null-c-string)*/

        /* DArenaVector reflects as mt_vector since
         * .xo-backlog/xo-reflect/issues/02.  The point of these two cases is
         * that printjson needed NO change to render it: print_generic_vector
         * keys on the metatype, not on std::vector, so describing the
         * container was the whole of the work.
         *
         * Before that specialisation existed, DArenaVector fell to
         * EstablishTdx's primary template and reflected as an atom, so this
         * rendered <error-json-printer-not-found ... metatype=mt_atomic>.
         */
        TEST_CASE("print-json-darena-vector", "[printjson][darenavector]") {
            PrintJson print_json;

            auto v = make_arena_vec<double>("utest.pj.dav");

            v.push_back(1.5);
            v.push_back(2.25);
            v.push_back(-3.0);

            std::stringstream ss;
            print_json.print(v, &ss);

            INFO("rendered: " << ss.str());

            REQUIRE(ss.str() == std::string("[1.5, 2.25, -3]"));
        } /*TEST_CASE(print-json-darena-vector)*/

        TEST_CASE("print-json-darena-vector-empty", "[printjson][darenavector]") {
            PrintJson print_json;

            auto v = make_arena_vec<double>("utest.pj.dav.empty");

            std::stringstream ss;
            print_json.print(v, &ss);

            /* empty, not null: n_child() reports SIZE, and a DArenaVector's
             * capacity is fixed at construction, so a non-zero capacity must
             * not show up here
             */
            REQUIRE(ss.str() == std::string("[]"));
        } /*TEST_CASE(print-json-darena-vector-empty)*/

        TEST_CASE("print-json-empty-struct", "[printjson]") {
            INFO(tag("s_init_evidence", s_init_evidence));

            StructReflector<TestStruct0> sr;

            sr.require_complete();

            TestStruct0 recd0;

            PrintJson print_json;

            TaggedPtr tp = Reflect::make_tp(&recd0);

            std::stringstream ss;

            print_json.print(tp, &ss);

            /* no members: still "_members_", so the shape never varies */
            REQUIRE(ss.str() == ("{\"_name_\": \"TestStruct0\"" + type_member<TestStruct0>()
                                 + ", \"_id_\": 1, \"_members_\": []}"));
        } /*TEST_CASE(print-json-empty-struct)*/

        namespace {
            struct TestStruct1 {
                std::int16_t i16_; std::uint16_t u16_;
                std::int32_t i32_; std::uint32_t u32_;
                std::int64_t i64_; std::uint64_t u64_;
                float f32_; double f64_;
                std::string s_;
            };
        }

        TEST_CASE("print-json-s1", "[printjson]") {
            INFO(tag("s_init_evidence", s_init_evidence));

            StructReflector<TestStruct1> sr;
            {
                REFLECT_MEMBER(sr, i16);
                REFLECT_MEMBER(sr, u16);
                REFLECT_MEMBER(sr, i32);
                REFLECT_MEMBER(sr, u32);
                REFLECT_MEMBER(sr, i64);
                REFLECT_MEMBER(sr, u64);
                REFLECT_MEMBER(sr, f32);
                REFLECT_MEMBER(sr, f64);
                REFLECT_MEMBER(sr, s);

                sr.require_complete();
            }

            TestStruct1 recd1{-1, 2, -3, 4, -5, 6, 1.23f, 4.56, "hello, world"};

            PrintJson print_json;

            TaggedPtr tp = Reflect::make_tp(&recd1);

            std::stringstream ss;

            print_json.print(tp, &ss);

            REQUIRE(ss.str() == ("{\"_name_\": \"TestStruct1\"" + type_member<TestStruct1>()
                                 + ", \"_id_\": 1, \"_members_\": ["
                                 + mentry<std::int16_t>("i16", "atomic", "-1") + ", "
                                 + mentry<std::uint16_t>("u16", "atomic", "2") + ", "
                                 + mentry<std::int32_t>("i32", "atomic", "-3") + ", "
                                 + mentry<std::uint32_t>("u32", "atomic", "4") + ", "
                                 + mentry<std::int64_t>("i64", "atomic", "-5") + ", "
                                 + mentry<std::uint64_t>("u64", "atomic", "6") + ", "
                                 + mentry<float>("f32", "atomic", "1.23") + ", "
                                 + mentry<double>("f64", "atomic", "4.56") + ", "
                                 + mentry<std::string>("s", "atomic", "\"hello, world\"") + "]}"));
        } /*TEST_CASE(print-json-s1)*/

        TEST_CASE("print-json-v1", "[printjson]") {
            INFO(tag("s_init_evidence", s_init_evidence));

            std::vector<double> v1{1, 2, 3};

            PrintJson print_json;

            TaggedPtr tp = Reflect::make_tp(&v1);

            std::stringstream ss;

            print_json.print(tp, &ss);

            REQUIRE(ss.str() == std::string("[1, 2, 3]"));
        } /*TEST_CASE(print-json-v1)*/

        namespace {
            /* a reflected enum, and one never reflected */
            enum class Mood { calm, cross };
            enum class Unlisted { x, y };

            /* reflected struct holding them */
            struct EnumHolder {
                Mood m_;
                Mood odd_;
            };

            struct UnlistedHolder {
                Unlisted u_;
            };

            void reflect_enum_types() {
                {
                    xo::reflect::EnumReflector<Mood> er;

                    if (er.is_incomplete()) {
                        REFLECT_ENUM(er, calm);
                        REFLECT_ENUM(er, cross);
                    }
                }
                {
                    StructReflector<EnumHolder> sr;

                    if (sr.is_incomplete()) {
                        REFLECT_MEMBER(sr, m);
                        REFLECT_MEMBER(sr, odd);
                    }
                }
                {
                    StructReflector<UnlistedHolder> sr;

                    if (sr.is_incomplete())
                        REFLECT_MEMBER(sr, u);
                }
            }
        }

        TEST_CASE("print-json-reflected-enum", "[printjson][enum]") {
            /* an enumerator's name, a json string; a value no enumerator
             * has, its integer, a json number (.xo-backlog/xo-reflect/issues/06)
             */
            reflect_enum_types();

            PrintJson print_json;
            EnumHolder h{Mood::cross, static_cast<Mood>(9)};

            std::stringstream ss;
            print_json.print(h, &ss);

            REQUIRE(ss.str() == ("{\"_name_\": \"EnumHolder\"" + type_member<EnumHolder>()
                                 + ", \"_id_\": 1, \"_members_\": ["
                                 + mentry<Mood>("m", "atomic", "\"cross\"") + ", "
                                 + mentry<Mood>("odd", "atomic", "9") + "]}"));
        } /*TEST_CASE(print-json-reflected-enum)*/

        TEST_CASE("print-json-unreflected-enum-member", "[printjson][enum]") {
            /* an enum never reflected cannot print: an error entry, so the
             * json stays valid
             */
            reflect_enum_types();

            PrintJson print_json;
            UnlistedHolder h{Unlisted::y};

            std::stringstream ss;
            print_json.print(h, &ss);

            REQUIRE(ss.str().find("\"_name_\": \"u\"") != std::string::npos);
            REQUIRE(ss.str().find("\"_error_\": \"type not reflected: ") != std::string::npos);
        } /*TEST_CASE(print-json-unreflected-enum-member)*/

        namespace {
            /* std::atomic members (.xo-backlog/xo-reflect/issues/04) */
            struct AtomicHolder {
                std::atomic<int> n_{5};
                std::atomic<bool> on_{true};
                std::atomic<Mood> mood_{Mood::calm};
            };

            void reflect_atomic_types() {
                reflect_enum_types();

                StructReflector<AtomicHolder> sr;

                if (sr.is_incomplete()) {
                    REFLECT_MEMBER(sr, n);
                    REFLECT_MEMBER(sr, on);
                    REFLECT_MEMBER(sr, mood);
                }
            }
        }

        TEST_CASE("print-json-std-atomic-members", "[printjson][stdatomic]") {
            /* each prints its load()ed value, under its declared type: an
             * atomic enum by its enumerator's name
             */
            reflect_atomic_types();

            PrintJson print_json;
            AtomicHolder h;
            h.n_.store(7);

            std::stringstream ss;
            print_json.print(h, &ss);

            REQUIRE(ss.str() == ("{\"_name_\": \"AtomicHolder\"" + type_member<AtomicHolder>()
                                 + ", \"_id_\": 1, \"_members_\": ["
                                 + mentry<std::atomic<int>>("n", "atomic", "7") + ", "
                                 + mentry<std::atomic<bool>>("on", "atomic", "true") + ", "
                                 + mentry<std::atomic<Mood>>("mood", "atomic", "\"calm\"") + "]}"));
        } /*TEST_CASE(print-json-std-atomic-members)*/

        namespace {
            /* a transparent wrapper, and a struct holding one
             * (.xo-backlog/xo-reflect/issues/04)
             */
            class Ticket {
            public:
                explicit Ticket(std::uint32_t n) : n_{n} {}
                static constexpr std::uint32_t Ticket::* n_address() { return &Ticket::n_; }
            private:
                std::uint32_t n_ = 0;
            };

            struct TicketHolder {
                Ticket t_{0};
            };

            void reflect_ticket_types() {
                {
                    xo::reflect::WrapperReflector<Ticket> wr;

                    if (wr.is_incomplete())
                        wr.reflect_wrapped(Ticket::n_address());
                }
                {
                    StructReflector<TicketHolder> sr;

                    if (sr.is_incomplete())
                        REFLECT_MEMBER(sr, t);
                }
            }
        }

        TEST_CASE("print-json-transparent-wrapper", "[printjson][wrapper]") {
            /* as the value it stands for -- not a struct holding it */
            reflect_ticket_types();

            PrintJson print_json;
            TicketHolder h{Ticket{42}};

            std::stringstream ss;
            print_json.print(h, &ss);

            REQUIRE(ss.str() == ("{\"_name_\": \"TicketHolder\"" + type_member<TicketHolder>()
                                 + ", \"_id_\": 1, \"_members_\": ["
                                 + mentry<Ticket>("t", "atomic", "42") + "]}"));
        } /*TEST_CASE(print-json-transparent-wrapper)*/

        /* also see tests:
         *   [option_util/utest/Px2.test.cpp]
         *   [option_util/utest/Size2.test.cpp]
         *   [option_util/utest/PxSize2.test.cpp]
         */
    } /*namespace ut */
} /*namespace xo*/


/* end StructReflector.test.cpp */
