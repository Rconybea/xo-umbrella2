/** @file PrintJsonGuard.test.cpp
 *
 *  the generic printer takes reflection-declared guards --
 *  .xo-backlog/xo-printjson/issues/09.
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "xo/printjson/PrintJson.hpp"
#include "xo/printjson/JsonPrinter.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/StructReflector.hpp>
#include <catch2/catch.hpp>
#include <atomic>
#include <memory>
#include <mutex>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace xo {
    using xo::json::JsonPrinter;
    using xo::json::JsonPrintState;
    using xo::json::PrintJson;
    using xo::reflect::GuardMode;
    using xo::reflect::Reflect;
    using xo::reflect::StructReflector;
    using xo::reflect::TaggedPtr;

    namespace ut {
        namespace {
            /** true iff another thread could take @p m now: this thread may
             *  hold it, and try_lock on a mutex the caller owns is undefined
             **/
            bool free_elsewhere(std::mutex & m) {
                bool retval = false;
                std::thread t([&m, &retval]() {
                    if (m.try_lock()) {
                        m.unlock();
                        retval = true;
                    }
                });
                t.join();
                return retval;
            }

            /** a member that, as it prints, notes which guards are held **/
            struct Probe {
                int tag_ = 0;
            };

            /** what one Probe saw as it printed **/
            struct ProbeSeen {
                int tag_;
                bool m1_free_;
                bool m2_free_;
            };

            /* the guards a ProbePrinter watches, and what it saw */
            std::mutex * s_m1 = nullptr;
            std::mutex * s_m2 = nullptr;
            std::vector<ProbeSeen> s_seen;

            class ProbePrinter : public JsonPrinter {
            public:
                virtual bool prints_object() const override { return false; }

                virtual void print_json(TaggedPtr tp, JsonPrintState & state) const override {
                    Probe * x = this->check_recover_native<Probe>(tp, state);

                    if (x) {
                        s_seen.push_back(ProbeSeen{x->tag_, free_elsewhere(*s_m1), free_elsewhere(*s_m2)});
                        *state.p_os() << x->tag_;
                    }
                }
            };

            /** guards out of declaration order: m1 guards x and z, m2 y **/
            struct Guarded {
                Probe x_;
                int free_ = 0;
                Probe y_;
                Probe z_;
                mutable std::mutex m1_;
                mutable std::mutex m2_;
            };

            /** two members kept equal by a writer, under m_ **/
            struct Pair {
                long a_ = 0;
                long b_ = 0;
                mutable std::mutex m_;
            };

            void reflect_types() {
                static bool s_once = []() {
                    {
                        StructReflector<Guarded> sr;
                        sr.reflect_member("x", &Guarded::x_).guarded_by(&Guarded::m1_);
                        sr.reflect_member("free", &Guarded::free_);
                        sr.reflect_member("y", &Guarded::y_).guarded_by(&Guarded::m2_);
                        sr.reflect_member("z", &Guarded::z_).guarded_by(&Guarded::m1_);
                    }
                    {
                        StructReflector<Pair> sr;
                        sr.reflect_member("a", &Pair::a_).guarded_by(&Pair::m_);
                        sr.reflect_member("b", &Pair::b_).guarded_by(&Pair::m_);
                    }
                    return true;
                }();
                (void)s_once;
            }

            void provide_probe_printer(PrintJson * pjson) {
                pjson->provide_printer(Reflect::require<Probe>(), std::make_unique<ProbePrinter>());
            }

            /** _members_ names of @p json's top-level object, in order: the
             *  lower-case _name_s (the object's own is its type's, Guarded)
             **/
            std::vector<std::string> member_names(std::string const & json) {
                std::vector<std::string> retval;
                std::regex re("\\{\"_name_\": \"([a-z]+)\", \"_canonical_type_\"");
                for (auto ix = std::sregex_iterator(json.begin(), json.end(), re);
                     ix != std::sregex_iterator(); ++ix)
                {
                    retval.push_back((*ix)[1].str());
                }
                return retval;
            }

            /** matches member @p name's entry up to @p rest, without
             *  running into the next entry (type names hold braces:
             *  {anonymous})
             **/
            std::regex entry_re(std::string const & name, std::string const & rest) {
                return std::regex("\"_name_\": \"" + name + "\"(?:(?!\"_name_\")[^])*?" + rest);
            }

            /** _value_ of member @p name in @p json, as a long **/
            long member_long(std::string const & json, std::string const & name) {
                std::regex re = entry_re(name, "\"_value_\": (-?[0-9]+)");
                std::smatch m;
                REQUIRE(std::regex_search(json, m, re));
                return std::stol(m[1].str());
            }
        } /*namespace*/

        TEST_CASE("print-json-guard-groups-and-holds", "[printjson][guard]") {
            reflect_types();

            PrintJson pjson;
            provide_probe_printer(&pjson);

            Guarded g;
            g.x_.tag_ = 1;
            g.y_.tag_ = 2;
            g.z_.tag_ = 3;

            s_m1 = &g.m1_;
            s_m2 = &g.m2_;
            s_seen.clear();

            std::stringstream ss;
            pjson.print(g, &ss);

            INFO(ss.str());

            /* unguarded first, then m1's group, then m2's */
            REQUIRE(member_names(ss.str()) == std::vector<std::string>{"free", "x", "z", "y"});

            /* each probe printed with exactly its own guard held */
            REQUIRE(s_seen.size() == 3);
            REQUIRE(s_seen[0].tag_ == 1);
            REQUIRE(!s_seen[0].m1_free_);
            REQUIRE(s_seen[0].m2_free_);
            REQUIRE(s_seen[1].tag_ == 3);
            REQUIRE(!s_seen[1].m1_free_);
            REQUIRE(s_seen[1].m2_free_);
            REQUIRE(s_seen[2].tag_ == 2);
            REQUIRE(s_seen[2].m1_free_);
            REQUIRE(!s_seen[2].m2_free_);

            /* and released */
            REQUIRE(free_elsewhere(g.m1_));
            REQUIRE(free_elsewhere(g.m2_));
        }

        TEST_CASE("print-json-guard-try-mode", "[printjson][guard]") {
            reflect_types();

            PrintJson pjson;
            provide_probe_printer(&pjson);
            pjson.assign_guard_mode(GuardMode::try_lock);

            Guarded g;
            g.free_ = 9;
            g.y_.tag_ = 2;

            s_m1 = &g.m1_;
            s_m2 = &g.m2_;
            s_seen.clear();

            std::string out;
            {
                /* this thread holds m1; the print runs elsewhere */
                std::lock_guard<std::mutex> lock(g.m1_);

                std::thread t([&]() {
                    std::stringstream ss;
                    pjson.print(g, &ss);
                    out = ss.str();
                });
                t.join();
            }

            INFO(out);

            /* m1's members unread, the rest printed */
            REQUIRE(out.find("{\"_name_\": \"x\"") != std::string::npos);
            REQUIRE(std::regex_search(out, entry_re("x", "\"_locked_\": true\\}")));
            REQUIRE(std::regex_search(out, entry_re("z", "\"_locked_\": true\\}")));
            REQUIRE(member_long(out, "free") == 9);
            REQUIRE(member_long(out, "y") == 2);
            /* only y's probe ran */
            REQUIRE(s_seen.size() == 1);
            REQUIRE(s_seen[0].tag_ == 2);
        }

        TEST_CASE("print-json-guard-consistent-under-writes", "[printjson][guard]") {
            /* a writer keeps a_ == b_ under m_; every print sees them equal */
            reflect_types();

            PrintJson pjson;
            Pair p;
            std::atomic<bool> stop{false};

            std::thread writer([&]() {
                for (long k = 1; !stop.load(); ++k) {
                    std::lock_guard<std::mutex> lock(p.m_);
                    p.a_ = k;
                    std::this_thread::yield();
                    p.b_ = k;
                }
            });

            int n_unequal = 0;
            long last = 0;
            for (int i = 0; i < 200; ++i) {
                std::stringstream ss;
                pjson.print(p, &ss);

                long a = member_long(ss.str(), "a");
                long b = member_long(ss.str(), "b");

                if (a != b)
                    ++n_unequal;
                last = a;
            }

            stop = true;
            writer.join();

            REQUIRE(n_unequal == 0);
            /* the writer did run meanwhile */
            REQUIRE(last > 0);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end PrintJsonGuard.test.cpp */
