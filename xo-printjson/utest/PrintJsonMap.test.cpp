/** @file PrintJsonMap.test.cpp
 *
 *  maps print through reflection -- .xo-backlog/xo-reflect/issues/05.
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "xo/printjson/PrintJson.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/StructReflector.hpp>
#include <catch2/catch.hpp>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>

namespace xo {
    using xo::json::PrintJson;
    using xo::reflect::StructReflector;

    namespace ut {
        namespace {
            struct Leaf {
                int v_ = 0;
            };

            /** a struct key: json cannot use it as an object key **/
            struct Point {
                int x_ = 0;
                int y_ = 0;
                bool operator<(Point const & p) const { return (x_ < p.x_) || ((x_ == p.x_) && (y_ < p.y_)); }
            };

            struct Maps {
                std::map<std::string, int> by_name_;
                std::unordered_map<std::uint64_t, std::unique_ptr<Leaf>> by_id_;
                std::map<Point, int> by_point_;
            };

            struct GuardedMap {
                std::map<std::string, int> m_;
                mutable std::mutex mutex_;
            };

            void reflect_types() {
                static bool s_once = []() {
                    {
                        StructReflector<Leaf> sr;
                        sr.reflect_member("v", &Leaf::v_);
                    }
                    {
                        StructReflector<Point> sr;
                        sr.reflect_member("x", &Point::x_);
                        sr.reflect_member("y", &Point::y_);
                    }
                    {
                        StructReflector<Maps> sr;
                        sr.reflect_member("by_name", &Maps::by_name_);
                        sr.reflect_member("by_id", &Maps::by_id_);
                        sr.reflect_member("by_point", &Maps::by_point_);
                    }
                    {
                        StructReflector<GuardedMap> sr;
                        sr.reflect_member("m", &GuardedMap::m_).guarded_by(&GuardedMap::mutex_);
                    }
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

            /** the "_value_" written for member @p name: from its key to the
             *  next member entry, or the end
             **/
            std::string value_of(std::string const & json, std::string const & name) {
                std::string const key = "\"_name_\": \"" + name + "\"";
                std::size_t i = json.find(key);
                REQUIRE(i != std::string::npos);
                std::size_t v = json.find("\"_value_\": ", i);
                REQUIRE(v != std::string::npos);
                return json.substr(v + 11);
            }
        } /*namespace*/

        TEST_CASE("print-json-map-string-keys", "[printjson][map]") {
            reflect_types();

            Maps x;
            x.by_name_ = {{"b", 2}, {"a", 1}};

            std::string s = print(x);

            INFO(s);
            /* a json object, in key order; metatype map */
            REQUIRE(contains(s, "\"_metatype_\": \"map\", \"_value_\": {\"a\": 1, \"b\": 2}"));
        }

        TEST_CASE("print-json-map-number-keys-sorted", "[printjson][map]") {
            reflect_types();

            Maps x;
            for (std::uint64_t k : {42, 7, 1000}) {
                x.by_id_[k] = std::make_unique<Leaf>();
                x.by_id_[k]->v_ = static_cast<int>(k);
            }

            std::string s = print(x);
            std::string v = value_of(s, "by_id");

            INFO(s);
            /* keys as strings, sorted as numbers; each Leaf placed here
             * (unique_ptr: owning), in full
             */
            std::size_t i7 = v.find("\"7\": {\"_name_\": \"Leaf\"");
            std::size_t i42 = v.find("\"42\": {\"_name_\": \"Leaf\"");
            std::size_t i1000 = v.find("\"1000\": {\"_name_\": \"Leaf\"");
            REQUIRE(i7 != std::string::npos);
            REQUIRE(i42 != std::string::npos);
            REQUIRE(i1000 != std::string::npos);
            REQUIRE(i7 < i42);
            REQUIRE(i42 < i1000);
            REQUIRE(!contains(s, "_unplaced_"));
        }

        TEST_CASE("print-json-map-struct-keys", "[printjson][map]") {
            /* a struct key: an array of entries */
            reflect_types();

            Maps x;
            x.by_point_[Point{2, 0}] = 20;
            x.by_point_[Point{1, 5}] = 15;

            std::string s = print(x);
            std::string v = value_of(s, "by_point");

            INFO(s);
            REQUIRE(v.substr(0, 11) == "[{\"_key_\": ");
            REQUIRE(std::regex_search(v, std::regex("^\\[\\{\"_key_\": \\{\"_name_\": \"Point\".*\"_value_\": 15\\}, \\{\"_key_\": ")));
            REQUIRE(contains(v, "\"_value_\": 20}]"));
        }

        TEST_CASE("print-json-map-empty", "[printjson][map]") {
            reflect_types();

            Maps x;

            std::string s = print(x);

            INFO(s);
            REQUIRE(value_of(s, "by_name").substr(0, 2) == "{}");
            REQUIRE(value_of(s, "by_id").substr(0, 2) == "{}");
            REQUIRE(value_of(s, "by_point").substr(0, 2) == "[]");
        }

        TEST_CASE("print-json-map-guarded", "[printjson][map]") {
            /* a map member under its guard, like any member */
            reflect_types();

            GuardedMap g;
            g.m_ = {{"k", 9}};

            std::string s = print(g);

            INFO(s);
            REQUIRE(contains(s, "\"_value_\": {\"k\": 9}"));
            /* released */
            REQUIRE(g.mutex_.try_lock());
            g.mutex_.unlock();
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end PrintJsonMap.test.cpp */
