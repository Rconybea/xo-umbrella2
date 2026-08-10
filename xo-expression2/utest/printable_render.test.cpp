/* @file printable_render.test.cpp
 *
 * author: Roland Conybeare, Aug 2026
 *
 * Phase C verification for xo-expression2's printers, bottom-up.  TypeRef
 * first: it is the subsystem's only leaf, depending on nothing else here.
 * Then DVariable, whose :typeref field nests it, then DVarRef.
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
 * DVariable, by contrast, IS a facet D-type: it is rendered here through
 * with_facet<APrintable>::mkobj(), needs a collector to exist at all, and picks
 * up the nested TypeRef through that Prettifier<TypeRef>.
 *
 * Expectations are OBSERVED, never predicted.
 */

#include "init_expression2.hpp"
#include <xo/expression2/TypeRef.hpp>
#include <xo/expression2/Variable.hpp>   /* convenience header: DVariable + its facet impls */
#include <xo/expression2/VarRef.hpp>     /* likewise DVarRef */
#include <xo/gc/X1Collector.hpp>
#include <xo/stringtable2/StringTable.hpp>
#include <xo/alloc2/CollectorTypeRegistry.hpp>
#include <xo/printable2/Printable.hpp>
#include <xo/indentlog2/print/toppstr.hpp>
#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/TypeDescr_ppdetail.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/testutil/UtestRehearser.hpp>
#include <xo/indentlog/print/ppstr.hpp>
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
    using xo::scm::DVariable;
    using xo::scm::DVarRef;
    using xo::scm::Binding;
    using xo::scm::DUniqueString;
    using xo::scm::StringTable;
    using xo::mm::CollectorTypeRegistry;
    using xo::mm::AAllocator;
    using xo::mm::ACollector;
    using xo::mm::DX1Collector;
    using xo::mm::X1CollectorConfig;
    using xo::mm::ArenaConfig;
    using xo::print::APrintable;
    using xo::facet::obj;
    using xo::facet::with_facet;
    using xo::reflect::Reflect;

    namespace ut {
        using xo::pp::PpConfig;
        using xo::pp::toppstr;
        using xo::pp::scope;
        using xo::pp::xtag;

        /** DVariable's APrintable facet is registered by SetupExpression2; the
         *  TypeRef cases do not need this, but obj<APrintable,DVariable> does.
         **/
        static InitEvidence s_init = InitSubsys<S_expression2_tag>::require();

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

            /** collector + string table for one DVariable case.
             *
             *  DVariable, unlike TypeRef, is a GC-allocated D-type: it needs an
             *  allocator to exist at all, and its APrintable facet needs
             *  SetupExpression2's registrations (see s_init below).  One
             *  collector per case, so a case cannot see another's arena.
             **/
            struct VarFixture {
                explicit VarFixture(const std::string & name)
                    : gc_{X1CollectorConfig{
                              .name_ = "printable_render." + name,
                              .arena_config_ = ArenaConfig{
                                  .size_ = 8192,
                                  .store_header_flag_ = true},
                              .object_types_z_ = 16384,
                              .gc_trigger_v_{{4096, 4096}},
                              .debug_flag_ = false}},
                      table_{1024}
                {
                    CollectorTypeRegistry::instance()
                        .install_types(with_facet<ACollector>::mkobj(&gc_));
                }

                obj<AAllocator> allocator() { return with_facet<AAllocator>::mkobj(&gc_); }

                /** @p name nullptr means an ANONYMOUS variable -- name_ is a
                 *  bare pointer with no non-null invariant, and legacy printed
                 *  the empty string for it.
                 **/
                DVariable * make_var(const char * name, Kind kind) {
                    const DUniqueString * sym = (name ? table_.intern(name) : nullptr);

                    return DVariable::make(this->allocator(), sym, make_typeref(kind));
                }

                /** @p path is the DEFINING variable's binding; DVarRef::make
                 *  derives its own via Binding::relative(link, path).
                 *
                 *  A sentinel (default-constructed) Binding is not reachable
                 *  this way -- Binding::relative asserts on it (Binding.cpp) --
                 *  so "{path}" is not among the cases below.
                 **/
                DVarRef * make_varref(const char * name, Binding path, std::int32_t link) {
                    const DUniqueString * sym = (name ? table_.intern(name) : nullptr);

                    DVariable * var = DVariable::make(this->allocator(), sym,
                                                      make_typeref(Kind::resolved),
                                                      path);

                    return DVarRef::make(this->allocator(), var, link);
                }

                DX1Collector gc_;
                StringTable table_;
            };

            /** MARGIN is the case variable, as for TypeRef above; NAME and the
             *  nested TypeRef's state vary too, since DVariable's printer has a
             *  branch on name_ and inherits TypeRef's on td_.
             **/
            struct Testcase_DVariable {
                Testcase_DVariable(const char * name,
                                   Kind kind,
                                   std::uint32_t margin,
                                   const char * label,
                                   const char * expect_deprecated,
                                   const char * expect_pretty)
                    : name_{name}, kind_{kind}, margin_{margin}, label_{label},
                      expect_deprecated_{expect_deprecated},
                      expect_pretty_{expect_pretty} {}

                /** nullptr -> anonymous variable **/
                const char * name_;
                Kind kind_;
                std::uint32_t margin_;
                /** distinguishes this case's arena **/
                const char * label_;
                /** OBSERVED via pretty_deprecated; delete at phase E **/
                std::string expect_deprecated_;
                /** OBSERVED via pretty; outlives phase E **/
                std::string expect_pretty_;
            };

            static std::vector<Testcase_DVariable> s_dvariable_v = {
                /* flat: identical, and the nested TypeRef arrives through
                 * Prettifier<TypeRef> rather than through a facet -- DVariable
                 * holds a TypeRef by value, not an obj<APrintable>.
                 */
                Testcase_DVariable("myvar", Kind::resolved, 200, "res200",
                                   "<DVariable :name \"myvar\" :typeref"
                                   " <TypeRef :id \"\" :td <TypeDescr :id N"
                                   " :canonical_name double :complete 1"
                                   " :metatype atomic>>>",
                                   "<DVariable :name \"myvar\" :typeref"
                                   " <TypeRef :id \"\" :td <TypeDescr :id N"
                                   " :canonical_name double :complete 1"
                                   " :metatype atomic>>>"),

                /* REVIEWED DIVERGENCE, the same one TypeRef pinned, now visible
                 * at TWO levels: the value of a broken field lands at
                 * indent+indent_width (legacy, 2) vs indent+tag_value_offset
                 * (ppsink, 1).  So the nested <TypeRef begins at column 4 vs 3,
                 * and its own fields at 6 vs 4.  Layout only; same tokens.
                 */
                Testcase_DVariable("myvar", Kind::resolved, 80, "res80",
                                   "<DVariable\n"
                                   "  :name \"myvar\"\n"
                                   "  :typeref\n"
                                   "    <TypeRef\n"
                                   "      :id \"\"\n"
                                   "      :td <TypeDescr :id N :canonical_name"
                                   " double :complete 1 :metatype atomic>>>",
                                   "<DVariable\n"
                                   "  :name \"myvar\"\n"
                                   "  :typeref\n"
                                   "   <TypeRef\n"
                                   "    :id \"\"\n"
                                   "    :td <TypeDescr :id N :canonical_name"
                                   " double :complete 1 :metatype atomic>>>"),

                /* the second half of that divergence: legacy's TypeDescr is a
                 * FlatSink render with no break points to offer, so it stays on
                 * one line however narrow the margin; ppsink breaks it.
                 */
                Testcase_DVariable("myvar", Kind::resolved, 40, "res40",
                                   "<DVariable\n"
                                   "  :name \"myvar\"\n"
                                   "  :typeref\n"
                                   "    <TypeRef\n"
                                   "      :id \"\"\n"
                                   "      :td\n"
                                   "        <TypeDescr :id N :canonical_name"
                                   " double :complete 1 :metatype atomic>>>",
                                   "<DVariable\n"
                                   "  :name \"myvar\"\n"
                                   "  :typeref\n"
                                   "   <TypeRef\n"
                                   "    :id \"\"\n"
                                   "    :td\n"
                                   "     <TypeDescr\n"
                                   "      :id N\n"
                                   "      :canonical_name double\n"
                                   "      :complete 1\n"
                                   "      :metatype atomic>>>"),

                /* unresolved typeref: short enough to stay flat at 200 ... */
                Testcase_DVariable("myvar", Kind::unresolved, 200, "unres200",
                                   "<DVariable :name \"myvar\" :typeref"
                                   " <TypeRef :id \"t:1\" :td null>>",
                                   "<DVariable :name \"myvar\" :typeref"
                                   " <TypeRef :id \"t:1\" :td null>>"),

                /* ... at 40 only the OUTER struct breaks, and the nested
                 * TypeRef still fits its line.  Identical, which is the useful
                 * part: the two stacks agree wherever nothing is forced.
                 */
                Testcase_DVariable("myvar", Kind::unresolved, 40, "unres40",
                                   "<DVariable\n"
                                   "  :name \"myvar\"\n"
                                   "  :typeref <TypeRef :id \"t:1\" :td null>>",
                                   "<DVariable\n"
                                   "  :name \"myvar\"\n"
                                   "  :typeref <TypeRef :id \"t:1\" :td null>>"),

                /* at 20 the nested TypeRef breaks too -- the indent divergence
                 * again, with no TypeDescr involved.
                 */
                Testcase_DVariable("myvar", Kind::unresolved, 20, "unres20",
                                   "<DVariable\n"
                                   "  :name \"myvar\"\n"
                                   "  :typeref\n"
                                   "    <TypeRef\n"
                                   "      :id \"t:1\"\n"
                                   "      :td null>>",
                                   "<DVariable\n"
                                   "  :name \"myvar\"\n"
                                   "  :typeref\n"
                                   "   <TypeRef\n"
                                   "    :id \"t:1\"\n"
                                   "    :td null>>"),

                /* a null name_ renders as "" -- NOT as nothing, and NOT as
                 * "null".  Both printers reach it through the same
                 * (name_ ? string_view(*name_) : string_view("")) branch, so
                 * this pins the branch rather than a formatting rule.
                 */
                Testcase_DVariable(nullptr, Kind::unresolved, 200, "anon200",
                                   "<DVariable :name \"\" :typeref"
                                   " <TypeRef :id \"t:1\" :td null>>",
                                   "<DVariable :name \"\" :typeref"
                                   " <TypeRef :id \"t:1\" :td null>>"),
            };

            /** DVarRef's own case variables: the DEFINING variable's binding,
             *  the link count, and the margin.  Its typeref is not printed, so
             *  it is held at Kind::resolved throughout and is not a variable.
             **/
            struct Testcase_DVarRef {
                Testcase_DVarRef(const char * name,
                                 Binding path,
                                 std::int32_t link,
                                 std::uint32_t margin,
                                 const char * label,
                                 const char * expect_deprecated,
                                 const char * expect_pretty)
                    : name_{name}, path_{path}, link_{link}, margin_{margin},
                      label_{label},
                      expect_deprecated_{expect_deprecated},
                      expect_pretty_{expect_pretty} {}

                const char * name_;
                Binding path_;
                std::int32_t link_;
                std::uint32_t margin_;
                const char * label_;
                /** OBSERVED via pretty_deprecated; delete at phase E **/
                std::string expect_deprecated_;
                /** OBSERVED via pretty; outlives phase E **/
                std::string expect_pretty_;
            };

            static std::vector<Testcase_DVarRef> s_dvarref_v = {
                /* flat, identical.  Note :name is NOT quoted -- legacy did not
                 * quote it here, though DVariable quotes its own :name.  That
                 * inconsistency is legacy's and is preserved deliberately;
                 * unifying the two would be an output-visible change wanting
                 * its own commit.
                 *
                 * :path is the point of this printer.  Binding has no
                 * Prettifier<> and no ppdetail<>, only an operator<<
                 * (Binding.hpp:58), so it takes ppsink's leaf FALLBACK -- empty
                 * primary template -> not string-like -> operator<<.  That the
                 * two renderings agree is the evidence the fallback fires, and
                 * it is worth pinning because a MISSING Prettifier<> is silent
                 * for any type that has an operator<< (TypeRef, having none,
                 * failed loudly instead).
                 */
                Testcase_DVarRef("myvar", Binding::local(3), 0, 200, "local0.200",
                                 "<DVarRef :name myvar :path {path:0:3}>",
                                 "<DVarRef :name myvar :path {path:0:3}>"),

                /* outer struct breaks; both values still fit their lines */
                Testcase_DVarRef("myvar", Binding::local(3), 0, 30, "local0.30",
                                 "<DVarRef\n"
                                 "  :name myvar\n"
                                 "  :path {path:0:3}>",
                                 "<DVarRef\n"
                                 "  :name myvar\n"
                                 "  :path {path:0:3}>"),

                /* margin 12: BOTH values break, so the known field-value column
                 * divergence (legacy 4, ppsink 3) shows up twice in one render.
                 */
                Testcase_DVarRef("myvar", Binding::local(3), 0, 12, "local0.12",
                                 "<DVarRef\n"
                                 "  :name\n"
                                 "    myvar\n"
                                 "  :path\n"
                                 "    {path:0:3}>",
                                 "<DVarRef\n"
                                 "  :name\n"
                                 "   myvar\n"
                                 "  :path\n"
                                 "   {path:0:3}>"),

                /* link 2: DVarRef::make composes its binding via
                 * Binding::relative(link, vardef->path()), so the rendered
                 * i_link is 2 rather than the defining variable's 0.  Pins the
                 * composition, not just the formatting.
                 */
                Testcase_DVarRef("myvar", Binding::local(3), 2, 200, "local2.200",
                                 "<DVarRef :name myvar :path {path:2:3}>",
                                 "<DVarRef :name myvar :path {path:2:3}>"),

                /* a global binding prints its own way ("{path:global:7}",
                 * Binding::print) and ignores the link -- Binding::relative
                 * returns the definition unchanged for globals.
                 */
                Testcase_DVarRef("g", Binding::global(7), 0, 200, "global.200",
                                 "<DVarRef :name g :path {path:global:7}>",
                                 "<DVarRef :name g :path {path:global:7}>"),

                /* margin 20: :name fits, :path does not -- the divergence on
                 * one field with the other left alone.
                 */
                Testcase_DVarRef("g", Binding::global(7), 0, 20, "global.20",
                                 "<DVarRef\n"
                                 "  :name g\n"
                                 "  :path\n"
                                 "    {path:global:7}>",
                                 "<DVarRef\n"
                                 "  :name g\n"
                                 "  :path\n"
                                 "   {path:global:7}>"),
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

        TEST_CASE("DVariable-render", "[printable][DVariable]")
        {
            REQUIRE(s_init.evidence());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "DVariable-render"));

                for (std::size_t i_tc = 0, n_tc = s_dvariable_v.size(); i_tc < n_tc; ++i_tc) {
                    const auto & tc = s_dvariable_v[i_tc];

                    VarFixture fx(tc.label_);

                    DVariable * var = fx.make_var(tc.name_, tc.kind_);
                    REQUIRE(var != nullptr);

                    /* the facet, not the raw pointer: this is how a DVariable
                     * is printed in anger, and it is the path phase D removes
                     * IPrintable::pretty(ppindentinfo) from.
                     */
                    auto p = with_facet<APrintable>::mkobj(var);

                    std::string deprecated = scrub_type_id(render_deprecated(p, tc.margin_));
                    std::string pretty = scrub_type_id(render_pretty(p, tc.margin_));

                    log && log(xtag("i_tc", i_tc), xtag("margin", tc.margin_),
                               xtag("deprecated", deprecated), xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
                    REHEARSE(rh, deprecated == tc.expect_deprecated_);
                }
            }
        }

        TEST_CASE("DVarRef-render", "[printable][DVarRef]")
        {
            REQUIRE(s_init.evidence());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "DVarRef-render"));

                for (std::size_t i_tc = 0, n_tc = s_dvarref_v.size(); i_tc < n_tc; ++i_tc) {
                    const auto & tc = s_dvarref_v[i_tc];

                    VarFixture fx(tc.label_);

                    DVarRef * vr = fx.make_varref(tc.name_, tc.path_, tc.link_);
                    REQUIRE(vr != nullptr);

                    auto p = with_facet<APrintable>::mkobj(vr);

                    std::string deprecated = render_deprecated(p, tc.margin_);
                    std::string pretty = render_pretty(p, tc.margin_);

                    log && log(xtag("i_tc", i_tc), xtag("margin", tc.margin_),
                               xtag("deprecated", deprecated), xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
                    REHEARSE(rh, deprecated == tc.expect_deprecated_);
                }
            }
        }

        /** the one case that CANNOT be pinned against both protocols.
         *
         *  DVarRef::pretty_deprecated does std::string_view(*(this->name()))
         *  with no null check, and a DVariable's name_ has no non-null
         *  invariant -- DVariable's own printer guards for exactly this.  So
         *  legacy is undefined here rather than merely different, and there is
         *  no legacy rendering to compare against.  DVarRef::pretty() guards,
         *  matching the sibling printer; this pins that guard.
         **/
        TEST_CASE("DVarRef-anon-render", "[printable][DVarRef]")
        {
            REQUIRE(s_init.evidence());

            VarFixture fx("anon");

            DVarRef * vr = fx.make_varref(nullptr, Binding::local(1), 0);
            REQUIRE(vr != nullptr);

            auto p = with_facet<APrintable>::mkobj(vr);

            CHECK(render_pretty(p, 200) == "<DVarRef :name  :path {path:0:1}>");
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end printable_render.test.cpp */
