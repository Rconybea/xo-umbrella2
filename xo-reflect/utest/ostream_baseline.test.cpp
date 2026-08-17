/* @file ostream_baseline.test.cpp
 *
 * Baseline pin for every rendering the xo-reflect ostream conversion will
 * touch: TypeDescrBase, TypeDescr, TypeId, Metatype, TaggedRcptr.
 *
 * Written BEFORE the conversion, deliberately.  xo-reflect has 34 consumers and
 * almost none of its output was pinned -- TypeDescr_pp.test.cpp:48 asserted an
 * eleven-character prefix and nothing else -- so a rendering change would have
 * reached every one of them silently.  Per CONVENTIONS.md, every expectation
 * here was READ off the current build rather than predicted; two incidents on
 * this migration came from predicting layout.
 *
 * Milestone: ostream-containment.  See .xo-backlog/xo-ppsink/issues/15.
 *
 * WHAT THIS IS FOR
 *
 * xo-reflect renders TypeDescrBase two ways -- separate hand-maintained
 * implementations of the same four fields:
 *
 *   TypeDescrBase::pretty(PpSink&)    TypeDescr.cpp:320   pretty_struct + field()
 *   TypeDescrBase::display(ostream&)  TypeDescr.cpp:337   os << xtag()
 *
 * Measured 2026-08-17: their TEXT is byte-identical.  They differ only in which
 * style slot colours the field names -- struct_tag_color (yellow) for field(),
 * tag_color (grey) for xtag() -- which is a deliberate distinction, not drift
 * (pretty_struct.hpp:106-109: "a struct field and a log tag" are different
 * things).  Under the utest harness's plain style neither emits colour at all,
 * so `reflect-baseline-colour-differs` forces a coloured style to see it.
 *
 * The conversion deletes display() and routes the ostream inserter through
 * pretty().  Text is unaffected; on a coloured terminal a TypeDescr's field
 * names move from log-tag grey to struct-field yellow, which is the correct
 * classification since a TypeDescr rendering IS a struct.  That is the only
 * output-visible consequence, and the colour test is where it is recorded --
 * when that test fails, the failure is the intended change and the test gets
 * updated rather than worked around.
 */

#include "xo/reflect/Reflect.hpp"
#include "xo/reflect/TypeDescr.hpp"
#include "xo/reflect/TaggedRcptr.hpp"
#include "xo/reflect/Metatype.hpp"
#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TypeDescr;
    using xo::reflect::TypeDescrBase;
    using xo::reflect::TaggedRcptr;
    using xo::reflect::Metatype;
    using xo::pp::FlatSink;

    namespace ut {
        namespace {
            /** drop ANSI SGR sequences, so the content pin does not break when
             *  the style changes.  Colour is pinned separately, on purpose.
             **/
            std::string strip_ansi(const std::string & s) {
                std::string out;
                for (std::size_t i = 0; i < s.size(); ++i) {
                    if (s[i] == '\033') {
                        while (i < s.size() && s[i] != 'm')
                            ++i;
                        continue;
                    }
                    out.push_back(s[i]);
                }
                return out;
            }

            /** the SGR sequences, in order, as a vector of e.g. "33" **/
            std::vector<std::string> ansi_codes(const std::string & s) {
                std::vector<std::string> out;
                for (std::size_t i = 0; i < s.size(); ++i) {
                    if (s[i] != '\033' || i + 1 >= s.size() || s[i + 1] != '[')
                        continue;
                    std::size_t j = i + 2;
                    std::string code;
                    while (j < s.size() && s[j] != 'm')
                        code.push_back(s[j++]);
                    out.push_back(code);
                    i = j;
                }
                return out;
            }

            std::string via_pretty(const TypeDescrBase & x) {
                std::stringstream ss;
                FlatSink sink(ss.rdbuf());
                sink.pp(x);
                sink.complete();
                std::string s = ss.str();
                if (!s.empty() && s.back() == '\n')   /* complete()'s newline */
                    s.pop_back();
                return s;
            }

            template <typename T>
            std::string via_ostream(const T & x) {
                std::stringstream ss;
                ss << x;
                return ss.str();
            }

            /** TypeDescr renders its own id, and ids are assigned in
             *  registration order -- which depends on what else this binary has
             *  reflected.  So build the expectation from the value rather than
             *  hardcoding it.  (Observed as 4 for int in an isolated probe;
             *  asserting 4 here would pass today and break the first time a test
             *  is added ahead of this one.)
             **/
            std::string expected_typedescr(TypeDescr td, const char * name) {
                return std::string("<TypeDescr :id ") + std::to_string(td->id().id())
                    + " :canonical_name " + name
                    + " :complete 1"
                    + " :metatype atomic>";
            }

            class Rc : public ref::Refcount {};
        } /*namespace*/

        TEST_CASE("reflect-baseline-typedescr-content", "[reflect-baseline]") {
            TypeDescr td_i = Reflect::require<int>();
            TypeDescr td_d = Reflect::require<double>();

            /* both paths, both types, content byte-exact once colour is removed */
            REQUIRE(strip_ansi(via_pretty(*td_i))  == expected_typedescr(td_i, "int"));
            REQUIRE(strip_ansi(via_ostream(*td_i)) == expected_typedescr(td_i, "int"));
            REQUIRE(strip_ansi(via_pretty(*td_d))  == expected_typedescr(td_d, "double"));
            REQUIRE(strip_ansi(via_ostream(*td_d)) == expected_typedescr(td_d, "double"));
        }

        TEST_CASE("reflect-baseline-pretty-and-display-agree-on-content",
                  "[reflect-baseline]") {
            /* THE claim that makes deleting display() safe.  If this ever fails,
             * the two hand-maintained renderings have drifted, and the deletion
             * is an output change rather than a cleanup.
             */
            TypeDescr td = Reflect::require<int>();

            REQUIRE(strip_ansi(via_pretty(*td)) == strip_ansi(via_ostream(*td)));
        }

        TEST_CASE("reflect-baseline-colour-differs", "[reflect-baseline]") {
            /* EXPECTED TO FAIL when display() is deleted -- see the file header.
             *
             * The utest harness runs under a plain style, so neither path emits
             * colour by default and the difference is invisible here.  Force a
             * coloured style to see it.  default_style_guard rather than
             * sink.with_style(): the ostream path goes through xtag's own
             * internally-built FlatSink, which can only be styled through the
             * defaults it copies (PpStyle.hpp:100-113).
             */
            xo::pp::default_style_guard colored(xo::pp::PpStyle::colored());

            TypeDescr td = Reflect::require<int>();

            auto pretty_codes = ansi_codes(via_pretty(*td));
            auto ostream_codes = ansi_codes(via_ostream(*td));

            REQUIRE(pretty_codes.size() == ostream_codes.size());
            REQUIRE(!pretty_codes.empty());

            /* NOT drift between two implementations -- a deliberate distinction.
             * field() inside a pretty_struct uses struct_tag_color (yellow);
             * xtag() uses tag_color (grey).  pretty_struct.hpp:106-109 spells out
             * why: "a struct field and a log tag" are different things.
             *
             * So deleting display() reclassifies TypeDescr's field names from log
             * tag to struct field, which is what they are.  The colour change is
             * a consequence of that, and only visible on a coloured terminal.
             */
            REQUIRE(pretty_codes.front() == "33");           /* struct_tag_color */
            REQUIRE(ostream_codes.front() == "38;5;245");    /* tag_color        */
        }

        TEST_CASE("reflect-baseline-typedescr-null", "[reflect-baseline]") {
            /* the two paths DISAGREE on null, and always have:
             *   operator<<(ostream&, const TypeDescrBase *)  -> "<nullptr>"
             *   Prettifier<TypeDescr>                        -> nothing at all
             * TypeDescr.hpp:566-572 records the second as deliberate.  Pinned so
             * the conversion has to decide rather than drift.
             */
            REQUIRE(via_ostream((TypeDescr)nullptr) == "<nullptr>");

            std::stringstream ss;
            FlatSink sink(ss.rdbuf());
            sink.pp((TypeDescr)nullptr);
            sink.complete();
            REQUIRE(ss.str() == "\n");
        }

        TEST_CASE("reflect-baseline-typeid", "[reflect-baseline]") {
            TypeDescr td = Reflect::require<int>();

            /* renders as the bare decimal, no framing */
            REQUIRE(via_ostream(td->id()) == std::to_string(td->id().id()));
        }

        TEST_CASE("reflect-baseline-metatype", "[reflect-baseline]") {
            /* every enumerator, so a Prettifier<Metatype> has to reproduce all
             * six -- including the "!" on invalid, which is easy to drop
             */
            REQUIRE(via_ostream(Metatype::mt_invalid)  == "invalid!");
            REQUIRE(via_ostream(Metatype::mt_atomic)   == "atomic");
            REQUIRE(via_ostream(Metatype::mt_pointer)  == "pointer");
            REQUIRE(via_ostream(Metatype::mt_vector)   == "vector");
            REQUIRE(via_ostream(Metatype::mt_struct)   == "struct");
            REQUIRE(via_ostream(Metatype::mt_function) == "function");
        }

        TEST_CASE("reflect-baseline-taggedrcptr", "[reflect-baseline]") {
            /* TaggedRcptr has no Prettifier and no coverage at all today. */
            rp<Rc> p(new Rc());
            TaggedRcptr tp(Reflect::require<Rc>(), p.get());

            std::string s = strip_ansi(via_ostream(tp));

            /* the addr field is a heap address, so pin the shape around it */
            const std::string prefix = "<TaggedRcptr :type ";
            REQUIRE(s.substr(0, prefix.size()) == prefix);
            REQUIRE(s.find(" :addr 0x") != std::string::npos);
            REQUIRE(s.back() == '>');

            /* and pin the type field exactly, which is the part that renders a
             * value rather than an address
             */
            REQUIRE(s.find(std::string(" :type ") + Reflect::require<Rc>()->canonical_name())
                    != std::string::npos);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end ostream_baseline.test.cpp */
