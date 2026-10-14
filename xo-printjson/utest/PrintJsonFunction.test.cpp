/** @file PrintJsonFunction.test.cpp
 *
 *  a function value -- std::function or function pointer -- prints as
 *  "set" / "empty" (.xo-backlog/xo-reflect/issues/09,
 *  .xo-backlog/std-function-reflection/spec.md section 2)
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "xo/printjson/PrintJson.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/StructReflector.hpp>
#include <xo/reflectutil/type_name.hpp>
#include <catch2/catch.hpp>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

namespace xo {
    using xo::json::PrintJson;
    using xo::reflect::StructReflector;
    using xo::reflect::type_name;

    namespace ut {
        namespace {
            int twice(double x) { return static_cast<int>(2 * x); }

            using Fn = std::function<int (double)>;
            using FnPtr = int (*)(double);

            struct Callables {
                Fn set_fn_ = [](double x) { return static_cast<int>(x); };
                Fn empty_fn_;
                FnPtr set_ptr_ = &twice;
                FnPtr null_ptr_ = nullptr;
            };

            void reflect_types() {
                static bool s_once = []() {
                    StructReflector<Callables> sr;
                    sr.reflect_member("set_fn", &Callables::set_fn_);
                    sr.reflect_member("empty_fn", &Callables::empty_fn_);
                    sr.reflect_member("set_ptr", &Callables::set_ptr_);
                    sr.reflect_member("null_ptr", &Callables::null_ptr_);
                    return true;
                }();
                (void)s_once;
            }

            template <typename T>
            std::string print(T const & x) {
                PrintJson pjson;
                std::stringstream ss;
                pjson.print(x, &ss);
                return ss.str();
            }

            bool contains(std::string const & s, std::string const & pat) {
                return s.find(pat) != std::string::npos;
            }

            /** the "_value_" written for member @p name **/
            std::string value_of(std::string const & json, std::string const & name) {
                std::string const key = "\"_name_\": \"" + name + "\"";
                std::size_t i = json.find(key);
                REQUIRE(i != std::string::npos);
                std::size_t v = json.find("\"_value_\": ", i);
                REQUIRE(v != std::string::npos);
                std::size_t e = json.find_first_of(",}", v + 11);
                return json.substr(v + 11, e - (v + 11));
            }
        } /*namespace*/

        TEST_CASE("function-members-print-set-or-empty", "[printjson][function]") {
            reflect_types();

            std::string s = print(Callables());

            INFO(s);
            REQUIRE(!contains(s, "<error-"));
            REQUIRE(!contains(s, "\"_error_\""));

            REQUIRE(value_of(s, "set_fn") == "\"set\"");
            REQUIRE(value_of(s, "empty_fn") == "\"empty\"");
            REQUIRE(value_of(s, "set_ptr") == "\"set\"");
            REQUIRE(value_of(s, "null_ptr") == "\"empty\"");

            /* the declared type -- the signature -- in the compiler's own
             * spelling (gcc and clang differ)
             */
            REQUIRE(contains(s, "\"_name_\": \"set_fn\", \"_canonical_type_\": \""
                                + std::string(type_name<Fn>()) + "\""));
            REQUIRE(contains(s, "\"_name_\": \"set_ptr\", \"_canonical_type_\": \""
                                + std::string(type_name<FnPtr>()) + "\""));
        }

        /* not a struct member: a vector element reaches print_node directly */
        TEST_CASE("a-vector-of-std-functions", "[printjson][function]") {
            std::vector<std::function<void ()>> v;
            v.push_back([]() {});
            v.push_back(std::function<void ()>());

            std::string s = print(v);

            INFO(s);
            REQUIRE(s == "[\"set\", \"empty\"]");
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end PrintJsonFunction.test.cpp */
