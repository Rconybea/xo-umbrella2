/** @file PrintJsonPretty.test.cpp
 *
 *  json into a PrettySink: objects and arrays break across lines when they
 *  do not fit the margin, and stay on one line when they do.  Flat output
 *  (a FlatSink) is unchanged by the structure -- every other test pins that.
 *  See .xo-backlog/xo-printjson/issues/01
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "xo/printjson/PrintJson.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/StructReflector.hpp>
#include <xo/indentlog2/print/PrettySink.hpp>
#include <xo/indentlog2/print/TempFlatSink.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <catch2/catch.hpp>
#include <string>
#include <vector>

namespace xo {
    using xo::json::PrintJson;
    using xo::pp::PpConfig;
    using xo::pp::PpStyle;
    using xo::pp::PrettySink;
    using xo::pp::TempFlatSink;
    using xo::reflect::Reflect;
    using xo::reflect::StructReflector;

    namespace ut {
        namespace {
            struct Leaf {
                int n_ = 1;
                std::vector<int> v_ = {10, 20, 30};
            };

            struct Tree {
                Leaf left_;
                Leaf right_;
                std::string label_ = "tree";
            };

            void reflect_types() {
                static bool s_once = []() {
                    {
                        StructReflector<Leaf> sr;
                        sr.reflect_member("n", &Leaf::n_);
                        sr.reflect_member("v", &Leaf::v_);
                    }
                    {
                        StructReflector<Tree> sr;
                        sr.reflect_member("left", &Tree::left_);
                        sr.reflect_member("right", &Tree::right_);
                        sr.reflect_member("label", &Tree::label_);
                    }
                    return true;
                }();
                (void)s_once;
            }

            std::string flat(Tree const & x) {
                PrintJson pjson;
                TempFlatSink out(PpStyle::json());
                pjson.print(x, &out.sink());
                return std::string(out.output());
            }

            std::string pretty(Tree const & x, std::uint32_t margin) {
                PrintJson pjson;
                PrettySink pp(PpConfig::scratch_plain(margin).with_style(PpStyle::json()),
                              nullptr /*out*/);
                pjson.print(x, &pp);
                return std::string(pp.output());
            }

            /** @p s without whitespace outside string literals **/
            std::string squeeze(std::string const & s) {
                std::string r;
                bool in_str = false;
                for (std::size_t i = 0; i < s.size(); ++i) {
                    char c = s[i];
                    if (in_str) {
                        r += c;
                        if (c == '\\' && i + 1 < s.size())
                            r += s[++i];
                        else if (c == '"')
                            in_str = false;
                    } else if (c == '"') {
                        in_str = true;
                        r += c;
                    } else if (c != ' ' && c != '\n') {
                        r += c;
                    }
                }
                return r;
            }
        } /*namespace*/

        /* fits: one line, identical to flat */
        TEST_CASE("pretty-json-wide-margin-is-flat", "[printjson][pretty]") {
            reflect_types();
            Tree x;

            REQUIRE(pretty(x, 100000) == flat(x));
        }

        /* does not fit: breaks, but is the same document */
        TEST_CASE("pretty-json-narrow-margin-breaks", "[printjson][pretty]") {
            reflect_types();
            Tree x;

            std::string p = pretty(x, 60);
            INFO(p);

            REQUIRE(p.find('\n') != std::string::npos);
            REQUIRE(squeeze(p) == squeeze(flat(x)));
        }

        /* objects and arrays nest by two columns, and each closing bracket
         * of a broken group starts its own line, at the opening line's indent
         */
        TEST_CASE("pretty-json-indents-by-nesting", "[printjson][pretty]") {
            reflect_types();
            Tree x;

            std::string p = pretty(x, 60);
            INFO(p);

            REQUIRE(p.rfind("\n}") == p.size() - 2);
            REQUIRE(p.find("\n  \"_members_\": [") != std::string::npos);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end PrintJsonPretty.test.cpp */
