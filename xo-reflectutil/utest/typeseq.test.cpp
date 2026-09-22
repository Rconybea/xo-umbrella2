/** @file typeseq.test.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  The name-keyed id table behind typeseq -- see
 *  .xo-backlog/xo-facet/issues/01.
 *
 *  What these pin is that identity does NOT depend on symbol merging.  The
 *  defect being fixed was invisible from inside one module: each pybind11
 *  extension module got a private copy of the id counter (hidden visibility
 *  demotes the GNU-unique symbol to a local one), so everything agreed with
 *  itself and disagreed with everything else.
 *
 *  A single-process test cannot reproduce two modules, so the cross-module
 *  claim is checked by `nm' -- see the regression check in the ticket, and the
 *  python assertion in xo-pyobject2. What CAN be pinned here is the property
 *  those rest on: one name, one id, from a table that is not a header static.
 *
 *  Expectations are OBSERVED, never predicted.
 */

#include "xo/reflectutil/typeseq.hpp"
#include <catch2/catch.hpp>
#include <string>

namespace xo {
    using xo::reflect::typeseq;
    using xo::reflect::typerecd;
    //using xo::reflect::typeseq_id_for;
    //using xo::reflect::typeseq_id_count;
    //using xo::reflect::typeseq_table_size;

    namespace ut {
        /* EXTERNAL linkage, deliberately -- at namespace xo::ut scope rather
         * than in an anonymous namespace.  These are the types that get
         * name-keyed.
         *
         * Worth stating because the first draft of this file put them in an
         * anonymous namespace along with DSeqHidden, and
         * `typeseq-id-agrees-with-the-table' then failed with 73 != 74: the
         * memoised id and a fresh typeseq_id_for() call on the same spelling
         * disagreed, which is EXACTLY what internal linkage is supposed to do.
         * The fixtures were wrong, not the table.
         */
        struct DSeqAlpha { int x_; };
        struct DSeqBeta  { double y_; };

        namespace {
            /* internal linkage, so it must NOT be name-keyed */
            struct DSeqHidden { int z_; };
        }

        TEST_CASE("same-name-gets-the-same-id", "[typeseq]") {
            /* the whole point.  Two calls spelling the same type get one id,
             * and they get it from a compiled function rather than from a
             * static this header would have duplicated per module.
             */
            typerecd a = typerecd::_by_name("xo::ut::DSeqProbe");
            typerecd b = typerecd::_by_name("xo::ut::DSeqProbe");

            REQUIRE(a.seqno() == b.seqno());

            /* and a different name gets a different one */
            typerecd c = typerecd::_by_name("xo::ut::DSeqOther");

            REQUIRE(a.seqno() != c.seqno());
        } /*TEST_CASE(same-name-gets-the-same-id)*/

        TEST_CASE("ids-are-dense-and-sequential", "[typeseq]") {
            /* TypeRegistry::_id2name indexes a DArenaVector by seqno(), so a
             * gap would be a hole in that vector
             */
            std::int32_t before = typerecd::id_count();

            std::int32_t a = typerecd::_by_name("xo::ut::DSeqDense1").seqno();
            std::int32_t b = typerecd::_by_name("xo::ut::DSeqDense2").seqno();
            std::int32_t c = typerecd::_by_name("xo::ut::DSeqDense3").seqno();

            REQUIRE(a == before);
            REQUIRE(b == before + 1);
            REQUIRE(c == before + 2);
            REQUIRE(typerecd::id_count() == before + 3);
        } /*TEST_CASE(ids-are-dense-and-sequential)*/

        TEST_CASE("internal-linkage-types-are-not-name-keyed", "[typeseq]") {
            /* two TUs' anonymous types SHARE a spelling and are different
             * types, so keying them together would hand them one id -- worse
             * than the bug this table fixes.  They draw from the counter
             * without being inserted.
             *
             * Both spellings, since gcc and clang disagree.
             */
            const char * gcc_spelling = "xo::ut::{anonymous}::DSeqWidget";
            const char * clang_spelling = "xo::ut::(anonymous namespace)::DSeqWidget";

            std::size_t rows_before = typerecd::table_z();

            std::int32_t a = typerecd::_by_name(gcc_spelling).seqno();
            std::int32_t b = typerecd::_by_name(gcc_spelling).seqno();
            std::int32_t c = typerecd::_by_name(clang_spelling).seqno();

            /* same spelling, DIFFERENT ids -- the opposite of the rule for
             * every other type
             */
            REQUIRE(a != b);
            REQUIRE(b != c);

            /* and no rows were added */
            REQUIRE(typerecd::table_z() == rows_before);
        } /*TEST_CASE(internal-linkage-types-are-not-name-keyed)*/

#ifdef BROKEN
        TEST_CASE("an-id-is-never-reassigned", "[typeseq]") {
            /* the invariant whose violation is SILENT.  Ids are cached in
             * per-type statics all over the process -- a trivial program
             * linking libxo_object2 draws 12 before main() -- so reassigning
             * one would invalidate every cache holding it, with nothing to
             * notice.
             *
             * Checked by drawing an id, forcing plenty of unrelated traffic,
             * then asking again.
             */
            std::int32_t first = typerecd::_by_name("xo::ut::DSeqStable").seqno();

            // this test violates lifetime rules
            for (int i = 0; i < 64; ++i)
                typeseq_id_for("xo::ut::DSeqFiller" + std::to_string(i));

            REQUIRE(typeseq_id_for("xo::ut::DSeqStable") == first);
        } /*TEST_CASE(an-id-is-never-reassigned)*/
#endif

        TEST_CASE("typeseq-id-agrees-with-the-table", "[typeseq]") {
            /* typeseq::id<T>() memoises in a per-type static; that cache is
             * still duplicated per module ON PURPOSE.  What must agree is the
             * cached value and what the table says for the same name.
             */
            std::int32_t via_typeseq = typeseq::id<DSeqAlpha>().seqno();
            std::int32_t via_table
                = typerecd::_by_name(xo::reflect::type_name<DSeqAlpha>()).seqno();

            REQUIRE(via_typeseq == via_table);

            /* stable across calls, and distinct per type */
            REQUIRE(typeseq::id<DSeqAlpha>().seqno() == via_typeseq);
            REQUIRE(typeseq::id<DSeqBeta>().seqno() != via_typeseq);

            /* the name typerecd reports is the type's own spelling */
            REQUIRE(typerecd::recd<DSeqAlpha>().name()
                    == xo::reflect::type_name<DSeqAlpha>());
        } /*TEST_CASE(typeseq-id-agrees-with-the-table)*/

        TEST_CASE("an-anonymous-type-still-gets-an-id", "[typeseq]") {
            /* exempt from name-keying, NOT from having an id: DSeqHidden is
             * used with the facet machinery in 12 test files across the tree
             */
            std::int32_t a = typeseq::id<DSeqHidden>().seqno();

            REQUIRE(a >= 0);
            /* memoised, so repeated asks are stable even though a fresh
             * typeseq_id_for() call on that spelling would not be
             */
            REQUIRE(typeseq::id<DSeqHidden>().seqno() == a);

            std::string name{xo::reflect::type_name<DSeqHidden>()};

            INFO("anonymous spelling: " << name);

            REQUIRE(((name.find("{anonymous}") != std::string::npos)
                     || (name.find("(anonymous namespace)") != std::string::npos)));
        } /*TEST_CASE(an-anonymous-type-still-gets-an-id)*/

    } /*namespace ut*/
} /*namespace xo*/

/* end typeseq.test.cpp */
