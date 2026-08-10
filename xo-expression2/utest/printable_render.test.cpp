/* @file printable_render.test.cpp
 *
 * author: Roland Conybeare, Aug 2026
 *
 * Phase C verification for xo-expression2's printers.  TypeRef first: it is
 * the subsystem's only leaf, depending on nothing else here.
 *
 * Follows the template in xo-object2/utest/printable_render.test.cpp -- see
 * .xo-backlog/xo-printable2/issues/01-aprintable-pretty-ppsink.md for why both
 * renderings are pinned rather than only asserted equal.
 *
 * TypeRef is the first converted printer that is NOT a facet D-type, so it
 * needs its own Prettifier<TypeRef> (TypeRef.hpp) exactly as it already needed
 * a print::ppdetail<TypeRef>; without one it would fall through Prettifier's
 * empty primary template to an operator<< it does not have.
 *
 * Expectations are OBSERVED, never predicted.
 */

#include <xo/expression2/TypeRef.hpp>
#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/TypeDescr_ppdetail.hpp>
#include <xo/indentlog2/print/toppstr.hpp>
#include <xo/indentlog/print/ppstr.hpp>
#include <xo/testutil/UtestRehearser.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <catch2/catch.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace xo {
    using xo::scm::TypeRef;
    using xo::scm::AType;
    using xo::facet::obj;
    using xo::reflect::Reflect;

    namespace ut {
        using xo::pp::PpConfig;
        using xo::pp::toppstr;
        using xo::pp::scope;
        using xo::pp::xtag;

        namespace {
            /** render @p x through the DEPRECATED two-pass protocol.
             *  DELETE AT PHASE E, with expect_deprecated_ and its REHEARSE.
             *
             *  Two color gates, as in xo-procedure2's copy of this helper: the
             *  :td field is a TypeDescr, whose legacy rendering reaches ppsink
             *  (TypeDescrBase::display streams xo::pp::xtag through a FlatSink),
             *  and ppsink reads its color from PpStyle rather than tag_config.
             **/
            template <typename T>
            std::string
            render_deprecated(const T & x, std::uint32_t margin) {
                xo::print::ppconfig ppc;
                ppc.right_margin_ = margin;

                bool orig_color = xo::tag_config::tag_color_enabled;
                xo::tag_config::tag_color_enabled = false;

                xo::pp::default_style_guard plain(xo::pp::PpStyle::plain());

                std::string retval = xo::toppstr2(ppc, x);

                xo::tag_config::tag_color_enabled = orig_color;

                return retval;
            }

            /** render @p x through pretty(PpSink&) **/
            template <typename T>
            std::string
            render_pretty(const T & x, std::uint32_t margin) {
                return toppstr(PpConfig::scratch_plain(margin), x);
            }

            /** replace the digits of ":id <n>" with "N".  See the identical
             *  helper in xo-procedure2/utest/printable_render.test.cpp: TypeId
             *  is a process-wide counter handed out in reflection order, so it
             *  moves when an unrelated test reflects a new type first.
             *
             *  This scrubs the NESTED TypeDescr's :id.  TypeRef's own :id is a
             *  type-variable NAME, is quoted, and stays pinned exactly.
             **/
            std::string scrub_type_id(std::string s) {
                const std::string key = ":id ";

                for (std::size_t i = s.find(key); i != std::string::npos; i = s.find(key, i+1)) {
                    std::size_t b = i + key.size();
                    std::size_t e = b;

                    while (e < s.size() && ::isdigit((unsigned char)s[e]))
                        ++e;

                    if (e > b)
                        s.replace(b, e - b, "N");
                }

                return s;
            }

            /** which of a TypeRef's two states to build.
             *
             *  The type variable name is supplied rather than generated:
             *  TypeRef::generate_unique() draws from a process-wide counter, so
             *  a generated name would move whenever an unrelated test made a
             *  TypeRef first.
             **/
            enum class Kind {
                /** id, no type description -- the normal pre-typecheck state **/
                unresolved,
                /** resolved via TypeRef::resolved(), so the id is EMPTY **/
                resolved,
                /** both: an id that later got resolved **/
                both,
            };

            TypeRef make_typeref(Kind kind) {
                switch (kind) {
                case Kind::unresolved:
                    return TypeRef(TypeRef::type_var::from_chars("t:1"), obj<AType>());
                case Kind::resolved:
                    return TypeRef::resolved(Reflect::require<double>());
                case Kind::both:
                    {
                        TypeRef retval(TypeRef::type_var::from_chars("t:2"),
                                       obj<AType>());
                        retval.resolve(Reflect::require<double>());
                        return retval;
                    }
                }

                return TypeRef();
            }

            /** MARGIN is the case variable, as elsewhere in phase C **/
            struct Testcase_TypeRef {
                Testcase_TypeRef(Kind kind,
                                 std::uint32_t margin,
                                 const char * expect_deprecated,
                                 const char * expect_pretty)
                    : kind_{kind}, margin_{margin},
                      expect_deprecated_{expect_deprecated},
                      expect_pretty_{expect_pretty} {}

                Kind kind_;
                std::uint32_t margin_;
                /** OBSERVED via pretty_deprecated; delete at phase E **/
                std::string expect_deprecated_;
                /** OBSERVED via pretty; outlives phase E **/
                std::string expect_pretty_;
            };

            static std::vector<Testcase_TypeRef> s_typeref_v = {
                /* An unresolved TypeRef prints ":td null".  ppsink has no
                 * equivalent of legacy cond(): Prettifier<TypeDescr> renders
                 * NOTHING for a null descriptor (TypeDescr_pp.hpp, deliberately
                 * -- changing it is an output-visible change to xo-reflect), so
                 * TypeRef::pretty() branches and supplies the word itself.
                 * Identical to legacy, which is the point.
                 */
                Testcase_TypeRef(Kind::unresolved, 200,
                                 "<TypeRef :id \"t:1\" :td null>",
                                 "<TypeRef :id \"t:1\" :td null>"),
                /* the struct breaks; fields still fit their own lines, and the
                 * struct-level indent agrees at 2.
                 */
                Testcase_TypeRef(Kind::unresolved, 20,
                                 "<TypeRef\n"
                                 "  :id \"t:1\"\n"
                                 "  :td null>",
                                 "<TypeRef\n"
                                 "  :id \"t:1\"\n"
                                 "  :td null>"),

                /* TypeRef::resolved() leaves the type variable name empty, and
                 * an empty id renders as "" -- NOT as nothing.  quot(), not
                 * unq(): legacy used xo::print::quot, which always quoted, and
                 * xo::pp::quot is its exact counterpart.  unq() would have
                 * rendered t:1 bare above and dropped these quotes here.
                 */
                Testcase_TypeRef(Kind::resolved, 200,
                                 "<TypeRef :id \"\" :td <TypeDescr :id N"
                                 " :canonical_name double :complete 1"
                                 " :metatype atomic>>",
                                 "<TypeRef :id \"\" :td <TypeDescr :id N"
                                 " :canonical_name double :complete 1"
                                 " :metatype atomic>>"),

                /* resolved: :td's value fits on its own line.  identical. */
                Testcase_TypeRef(Kind::both, 80,
                                 "<TypeRef\n"
                                 "  :id \"t:2\"\n"
                                 "  :td <TypeDescr :id N :canonical_name double"
                                 " :complete 1 :metatype atomic>>",
                                 "<TypeRef\n"
                                 "  :id \"t:2\"\n"
                                 "  :td <TypeDescr :id N :canonical_name double"
                                 " :complete 1 :metatype atomic>>"),

                /* REVIEWED DIVERGENCE, both halves already settled by
                 * DPrimitive (xo-procedure2) and reappearing here because :td
                 * is again a TypeDescr:
                 *
                 * 1. the value of a broken field lands in column 4 under legacy
                 *    (indent 2 + indent_width 2) and column 3 under ppsink
                 *    (indent 2 + tag_value_offset 1).
                 * 2. legacy's :td stays on ONE line at any margin -- its legacy
                 *    path is already a FlatSink and has no break points to
                 *    offer -- while ppsink breaks the nested struct.
                 */
                Testcase_TypeRef(Kind::both, 40,
                                 "<TypeRef\n"
                                 "  :id \"t:2\"\n"
                                 "  :td\n"
                                 "    <TypeDescr :id N :canonical_name double"
                                 " :complete 1 :metatype atomic>>",
                                 "<TypeRef\n"
                                 "  :id \"t:2\"\n"
                                 "  :td\n"
                                 "   <TypeDescr\n"
                                 "    :id N\n"
                                 "    :canonical_name double\n"
                                 "    :complete 1\n"
                                 "    :metatype atomic>>"),

                /* margin 20: legacy is UNCHANGED from margin 40, having nothing
                 * left to give.  ppsink degrades one step further, folding the
                 * nested fields' values onto their own lines.
                 */
                Testcase_TypeRef(Kind::both, 20,
                                 "<TypeRef\n"
                                 "  :id \"t:2\"\n"
                                 "  :td\n"
                                 "    <TypeDescr :id N :canonical_name double"
                                 " :complete 1 :metatype atomic>>",
                                 "<TypeRef\n"
                                 "  :id \"t:2\"\n"
                                 "  :td\n"
                                 "   <TypeDescr\n"
                                 "    :id N\n"
                                 "    :canonical_name\n"
                                 "     double\n"
                                 "    :complete 1\n"
                                 "    :metatype\n"
                                 "     atomic>>"),
            };
        } /*namespace*/

        TEST_CASE("TypeRef-render", "[printable][TypeRef]")
        {
            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "TypeRef-render"));

                for (std::size_t i_tc = 0, n_tc = s_typeref_v.size(); i_tc < n_tc; ++i_tc) {
                    const auto & tc = s_typeref_v[i_tc];

                    TypeRef tr = make_typeref(tc.kind_);

                    std::string deprecated = scrub_type_id(render_deprecated(tr, tc.margin_));
                    std::string pretty = scrub_type_id(render_pretty(tr, tc.margin_));

                    log && log(xtag("i_tc", i_tc), xtag("margin", tc.margin_),
                               xtag("deprecated", deprecated), xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
                    REHEARSE(rh, deprecated == tc.expect_deprecated_);
                }
            }
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end printable_render.test.cpp */
