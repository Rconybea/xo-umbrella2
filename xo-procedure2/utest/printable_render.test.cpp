/* @file printable_render.test.cpp
 *
 * author: Roland Conybeare, Aug 2026
 *
 * Phase C verification for xo-procedure2's printer: Primitive<Fn>, i.e. the
 * DPrimitive_gco_* family.
 *
 * Follows the template in xo-object2/utest/printable_render.test.cpp -- see
 * .xo-backlog/xo-printable2/issues/01-aprintable-pretty-ppsink.md for why both
 * renderings are pinned rather than only asserted equal.
 *
 * Primitive is the first converted STRUCT whose field values are wide enough
 * to leave the line, so it is the first place two things become visible:
 *
 *  - a broken field's value indents by tag_value_offset (1) under ppsink and
 *    by a full indent_width (2) under legacy, so the value sits one column
 *    left of where legacy put it;
 *  - the :td field is a TypeDescr, whose LEGACY rendering already goes through
 *    ppsink -- TypeDescrBase::display(ostream) streams xo::pp::xtag through a
 *    FlatSink -- so it cannot break at all, at any margin, while the ppsink
 *    rendering folds it.
 *
 * Expectations are OBSERVED, never predicted.
 */

#include <xo/procedure2/ObjectPrimitives.hpp>
#include <xo/procedure2/detail/IPrintable_DPrimitive_gco_0.hpp>
#include <xo/procedure2/init_procedure2.hpp>
#include <xo/stringtable2/StringTable.hpp>
#include <xo/alloc2/arena/IAllocator_DArena.hpp>
#include <xo/printable2/Printable.hpp>
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
    using xo::scm::ObjectPrimitives;
    using xo::scm::DPrimitive_gco_0;
    using xo::scm::StringTable;
    using xo::print::APrintable;
    using xo::mm::AAllocator;
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;
    using xo::facet::with_facet;
    using xo::facet::obj;

    namespace ut {
        using xo::pp::PpConfig;
        using xo::pp::toppstr;
        using xo::pp::scope;
        using xo::pp::xtag;

        namespace {
            /** render @p x through the DEPRECATED two-pass protocol.
             *  DELETE AT PHASE E, with expect_deprecated_ and its REHEARSE.
             *
             *  Two color gates, not one.  xo-object2's copy of this helper
             *  needed only tag_config::tag_color_enabled, because everything it
             *  rendered stayed inside legacy indentlog.  Here the :td field is
             *  a TypeDescr, whose legacy path reaches ppsink (see the file
             *  comment), and ppsink reads its color from PpStyle -- so without
             *  the style guard the "expected" strings would have to embed ANSI
             *  escapes for the nested fields and not for the outer ones.
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
                return toppstr(PpConfig::scratch(margin), x);
            }

            /** replace the digits of ":id <n>" with "N".
             *
             *  TypeId is a process-wide counter handed out in reflection order,
             *  so the :id inside :td depends on how many OTHER types this
             *  binary happens to reflect first -- adding an unrelated test file
             *  would change it.  It is xo-reflect's field, pinned by
             *  xo-reflect's own tests; scrubbing it here keeps every other
             *  column of the layout pinned exactly.
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

            /** the primitive is fixed; MARGIN is the case variable **/
            struct Testcase_Primitive {
                Testcase_Primitive(std::uint32_t margin,
                                   const char * expect_deprecated,
                                   const char * expect_pretty)
                    : margin_{margin},
                      expect_deprecated_{expect_deprecated},
                      expect_pretty_{expect_pretty} {}

                std::uint32_t margin_;
                /** OBSERVED via pretty_deprecated; delete at phase E **/
                std::string expect_deprecated_;
                /** OBSERVED via pretty; outlives phase E **/
                std::string expect_pretty_;
            };

            /** ObjectPrimitives::make_cwd_pm: name "cwd", Fn a plain function
             *  pointer.  The :canonical_name of that function type is 78
             *  columns wide all by itself, which is what makes the margins
             *  below interesting.
             *
             *  :fn 1 in BOTH renderings is not a typo and not a divergence: a
             *  function pointer has no operator<<, so it converts to bool.
             *  Legacy printed 1 too.  Recorded here so the next reader does not
             *  "fix" it as ppsink damage.
             **/
            static std::vector<Testcase_Primitive> s_primitive_v = {
                /* fits: identical, character for character */
                Testcase_Primitive(200,
                                   "<Primitive<Fn> :name cwd"
                                   " :td <TypeDescr :id N :canonical_name"
                                   " xo::facet::obj<xo::mm::AGCObject>"
                                   " (*)(xo::facet::obj<xo::scm::ARuntimeContext>)"
                                   " :complete 1 :metatype function> :fn 1>",
                                   "<Primitive<Fn> :name cwd"
                                   " :td <TypeDescr :id N :canonical_name"
                                   " xo::facet::obj<xo::mm::AGCObject>"
                                   " (*)(xo::facet::obj<xo::scm::ARuntimeContext>)"
                                   " :complete 1 :metatype function> :fn 1>"),

                /* REVIEWED DIVERGENCE, two of them, both deliberate:
                 *
                 * 1. the value of a broken field lands in column 4 under
                 *    legacy (indent 2 + indent_width 2) and column 3 under
                 *    ppsink (indent 2 + tag_value_offset 1).
                 * 2. legacy's :td stays on one 143-column line because its
                 *    legacy path is already a FlatSink and has no break points
                 *    to offer; ppsink folds it.
                 */
                Testcase_Primitive(80,
                                   "<Primitive<Fn>\n"
                                   "  :name cwd\n"
                                   "  :td\n"
                                   "    <TypeDescr :id N :canonical_name"
                                   " xo::facet::obj<xo::mm::AGCObject>"
                                   " (*)(xo::facet::obj<xo::scm::ARuntimeContext>)"
                                   " :complete 1 :metatype function>\n"
                                   "  :fn 1>",

                                   "<Primitive<Fn>\n"
                                   "  :name cwd\n"
                                   "  :td\n"
                                   "   <TypeDescr\n"
                                   "    :id N\n"
                                   "    :canonical_name\n"
                                   "     xo::facet::obj<xo::mm::AGCObject>"
                                   " (*)(xo::facet::obj<xo::scm::ARuntimeContext>)\n"
                                   "    :complete 1\n"
                                   "    :metatype function>\n"
                                   "  :fn 1>"),

                /* margin 20: legacy is UNCHANGED from margin 80 -- it has
                 * nothing left to give.  ppsink degrades one step further,
                 * folding :metatype's value onto its own line.  The
                 * :canonical_name value never folds in either: it is one
                 * atomic token, wider than any margin here.
                 */
                Testcase_Primitive(20,
                                   "<Primitive<Fn>\n"
                                   "  :name cwd\n"
                                   "  :td\n"
                                   "    <TypeDescr :id N :canonical_name"
                                   " xo::facet::obj<xo::mm::AGCObject>"
                                   " (*)(xo::facet::obj<xo::scm::ARuntimeContext>)"
                                   " :complete 1 :metatype function>\n"
                                   "  :fn 1>",

                                   "<Primitive<Fn>\n"
                                   "  :name cwd\n"
                                   "  :td\n"
                                   "   <TypeDescr\n"
                                   "    :id N\n"
                                   "    :canonical_name\n"
                                   "     xo::facet::obj<xo::mm::AGCObject>"
                                   " (*)(xo::facet::obj<xo::scm::ARuntimeContext>)\n"
                                   "    :complete 1\n"
                                   "    :metatype\n"
                                   "     function>\n"
                                   "  :fn 1>"),
            };
        }

        static InitEvidence s_init_render = InitSubsys<S_procedure2_tag>::require();

        TEST_CASE("DPrimitive-render", "[printable][DPrimitive]")
        {
            REQUIRE(s_init_render.evidence());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "DPrimitive-render"));

                for (std::size_t i_tc = 0, n_tc = s_primitive_v.size(); i_tc < n_tc; ++i_tc) {
                    const auto & tc = s_primitive_v[i_tc];

                    ArenaConfig cfg { .name_ = "utest.primitive." + std::to_string(i_tc),
                                      .size_ = 64*1024 };
                    DArena arena = DArena::map(cfg);
                    auto alloc = with_facet<AAllocator>::mkobj(&arena);

                    auto stbl = StringTable(1024 /*hint_max_capacity*/,
                                            false /*!debug_flag*/);

                    DPrimitive_gco_0 * pm = ObjectPrimitives::make_cwd_pm(alloc, &stbl);

                    REQUIRE(pm != nullptr);

                    auto p = with_facet<APrintable>::mkobj(pm);

                    std::string deprecated = scrub_type_id(render_deprecated(p, tc.margin_));
                    std::string pretty = scrub_type_id(render_pretty(p, tc.margin_));

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
