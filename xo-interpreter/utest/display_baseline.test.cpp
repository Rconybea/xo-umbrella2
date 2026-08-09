/** @file display_baseline.test.cpp
 *
 *  author: Roland Conybeare, Aug 2026
 *
 *  Pin the rendered form of xo-interpreter's xo::gc::Object subclasses, ahead
 *  of the display(std::ostream&) -> pretty(PpSink&) conversion
 *  (.xo-backlog/xo-alloc/issues/01-object-pretty-ppsink.md).
 *
 *  The conversion is meant to change no output; this is how that is checked.
 *  Companion files: xo-alloc/utest/display_baseline.test.cpp,
 *  xo-object/utest/display_baseline.test.cpp.
 *
 *  Expectations here were read off observed output, not predicted -- see
 *  .xo-backlog/CONVENTIONS.md on measuring before asserting.
 **/

#include "xo/interpreter/LocalEnv.hpp"
#include "xo/interpreter/init_interpreter.hpp"
#include <xo/object/Integer.hpp>
#include <xo/alloc/alloc_ostream.hpp>   /* os << gp<Object> */
#include <xo/alloc/ArenaAlloc.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <string>

namespace xo {
    using xo::scm::LocalEnv;
    using xo::gc::ArenaAlloc;

    namespace ut {
        static InitEvidence s_dbase_init = (InitSubsys<S_interpreter_tag>::require());

        namespace {
            template <typename T>
            std::string
            rendered(gp<T> x) {
                std::stringstream ss;
                ss << gp<Object>::from(x);
                return ss.str();
            }
        } /*namespace*/

        TEST_CASE("display-baseline-localenv", "[display_baseline]") {
            constexpr bool c_debug_flag = false;

            up<ArenaAlloc> alloc = ArenaAlloc::make("utest", 16384, c_debug_flag);
            ArenaAlloc * mm = alloc.get();
            Object::mm = mm;

            /* LocalEnv::display() is:
             *   os << "<local-env" << xtag("n", slot_v_.size()) ... << ">"
             * The slot loop is #ifdef NOT_YET, so only the arity shows today.
             * Pinned at several arities so the conversion cannot quietly
             * enable or drop that.
             */
            REQUIRE(rendered(LocalEnv::make(mm, nullptr, nullptr, 0))
                    == "<local-env :n 0>");
            REQUIRE(rendered(LocalEnv::make(mm, nullptr, nullptr, 2))
                    == "<local-env :n 2>");
        }

        /* NOT PINNED HERE, and deliberately said out loud rather than left to
         * look like coverage:
         *
         * - GlobalEnv    -- GlobalEnv::make_empty(mm, symtab) needs a
         *                   GlobalSymtab; no cheap fixture exists in this utest.
         *                   Body is: "<global-env" << xtag("n", size) << ">".
         * - VsmStackFrame -- VsmStackFrame::make(mm, parent, n_slot, cont)
         *                   needs a VsmInstr*.
         *                   Body is: "<vsm-stack-frame" << xtag("n", size) ...
         * - ExpressionBoxed -- bare `os << contents_`, delegating wholly to the
         *                   boxed expression's own rendering.
         *
         * All three follow the same xtag-in-angle-brackets shape that LocalEnv
         * pins above, so a conversion that keeps LocalEnv byte-identical is
         * very likely to keep these so too -- but that is an inference, not a
         * measurement. Whoever converts them should render one instance by hand
         * and compare, per .xo-backlog/CONVENTIONS.md.
         *
         * ExpressionBoxed is the one with a distinct property worth checking by
         * hand: it must keep DELEGATING to contents_, not acquire a wrapper of
         * its own.
         */
    } /*namespace ut*/
} /*namespace xo*/

/* end display_baseline.test.cpp */
