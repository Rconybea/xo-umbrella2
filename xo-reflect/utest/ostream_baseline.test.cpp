/* @file ostream_baseline.test.cpp
 *
 * Rendering pin for xo-reflect: TypeDescrBase, TypeDescr, TypeId, Metatype,
 * TaggedRcptr.
 *
 * Written BEFORE the ostream conversion, deliberately.  xo-reflect has 34
 * consumers and almost none of its output was pinned -- TypeDescr_pp.test.cpp:48
 * asserted an eleven-character prefix and nothing else -- so a rendering change
 * would have reached every one of them silently.  Per CONVENTIONS.md, every
 * expectation here was READ off the current build rather than predicted; two
 * incidents on this migration came from predicting layout.
 *
 * Milestone: ostream-containment.  See .xo-backlog/xo-ppsink/issues/15.
 *
 * WHAT THIS WAS FOR, AND WHAT IT IS NOW  (rewritten 2026-08-22, rule 6)
 *
 * Until the conversion, xo-reflect rendered TypeDescrBase two ways -- separate
 * hand-maintained implementations of the same four fields:
 *
 *   TypeDescrBase::pretty(PpSink&)     pretty_struct + field()
 *   TypeDescrBase::display(ostream&)   os << xtag()
 *
 * so this file held them side by side and asserted they agreed.  Measured
 * 2026-08-17 their TEXT was byte-identical; they differed only in which style
 * slot coloured the field names (struct_tag_color for field(), tag_color for
 * xtag()), a deliberate distinction rather than drift.  Two tests existed for
 * that comparison: `reflect-baseline-pretty-and-display-agree-on-content` and
 * `reflect-baseline-colour-differs`.
 *
 * BOTH WERE DELETED 2026-08-22.  display() and every ostream inserter in
 * xo-reflect are gone; PpSink is the only rendering path the subsystem offers,
 * so there is no second implementation to compare against and the two tests
 * had become, respectively, a tautology and an assertion about a colour slot
 * nothing uses any more.
 *
 * WHAT THAT COSTS, stated rather than left implicit: nothing now pins
 * xo-reflect's rendering against an independent implementation, because there
 * isn't one.  The content pins below are absolute -- they assert exact strings
 * read off the build -- which catches a change but cannot tell you whether the
 * change was intended.  That is the normal situation for a rendering test and
 * was NOT the situation while display() existed.
 *
 * The conversion turned out to be text-preserving: every content pin here
 * passed unchanged through it.  The one output-visible change is null
 * TypeDescr, which now renders "<nullptr>" -- see the CHANGED note on
 * Prettifier<TypeDescr> in TypeDescr.hpp, and `reflect-baseline-typedescr-null`
 * below.
 *
 * NOTE the filename is now a misnomer: there is no ostream and no baseline.
 * Kept because it is what the milestone tickets refer to.
 */

#include "xo/reflect/Reflect.hpp"
#include "xo/reflect/TypeDescr.hpp"
#include "xo/reflect/TaggedRcptr.hpp"
#include "xo/reflect/Metatype.hpp"
#include <xo/ppsink/FlatSink.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <string>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TypeDescr;
    using xo::reflect::TypeDescrBase;
    using xo::reflect::TaggedRcptr;
    using xo::reflect::Metatype;
    using xo::pp::FlatSink;
    using xo::pp::tostr;

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
                return tostr(x);
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

        TEST_CASE("reflect-baseline-typedescr-null", "[reflect-baseline]") {
            /* the two paths USED TO disagree on null:
             *   operator<<(ostream&, const TypeDescrBase *)  -> "<nullptr>"
             *   Prettifier<TypeDescr>                        -> nothing at all
             * The inserter is gone, and Prettifier<TypeDescr> adopted its
             * "<nullptr>" rather than inheriting silence by default -- see the
             * CHANGED note at that specialization.  So there is one behaviour
             * now, and this pins it.
             */
            REQUIRE(via_ostream((TypeDescr)nullptr) == "<nullptr>");

            std::stringstream ss;
            FlatSink sink(ss.rdbuf());
            sink.pp((TypeDescr)nullptr);
            sink.complete();
            REQUIRE(ss.str() == "<nullptr>\n");
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
