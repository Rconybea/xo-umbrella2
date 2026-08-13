/* @file printable_render.test.cpp
 *
 * author: Roland Conybeare, Aug 2026
 *
 * Phase C verification for xo-expression2's printers, bottom-up.  TypeRef
 * first: it is the subsystem's only leaf, depending on nothing else here.
 * Then DVariable, whose :typeref field nests it, then DVarRef, then
 * DGlobalSymtab, then DConstant, then DIfElseExpr, then DSequenceExpr.
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
#include <xo/expression2/ApplyExpr.hpp>     /* likewise DApplyExpr */
#include <xo/expression2/Constant.hpp>      /* likewise DConstant */
#include <xo/expression2/DefineExpr.hpp>    /* likewise DDefineExpr */
#include <xo/expression2/GlobalSymtab.hpp>  /* likewise DGlobalSymtab */
#include <xo/expression2/IfElseExpr.hpp>     /* likewise DIfElseExpr */
#include <xo/expression2/LambdaExpr.hpp>     /* likewise DLambdaExpr */
#include <xo/expression2/Typename.hpp>      /* likewise DTypename */
#include <xo/expression2/LocalSymtab.hpp>    /* likewise DLocalSymtab */
#include <xo/expression2/SequenceExpr.hpp>   /* likewise DSequenceExpr */
#include <xo/expression2/TypeRef.hpp>
#include <xo/expression2/VarRef.hpp>       /* likewise DVarRef */
#include <xo/expression2/Variable.hpp>     /* convenience header: DVariable + its facet impls */
#include <xo/type/AtomicType.hpp>            /* DAtomicType, for DLocalSymtab's :types */
#include <xo/object2/Float.hpp>
#include <xo/object2/Integer.hpp>
#include <xo/gc/X1Collector.hpp>
#include <xo/stringtable2/StringTable.hpp>
#include <xo/alloc2/arena/IAllocator_DArena.hpp>
#include <xo/alloc2/CollectorTypeRegistry.hpp>
#include <xo/printable2/Printable.hpp>
#include <xo/indentlog2/print/toppstr.hpp>
#include <xo/reflect/Reflect.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/testutil/UtestRehearser.hpp>
#include <xo/arena/ArenaHashMapConfig.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <catch2/catch.hpp>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <cctype>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace xo {
    using xo::scm::TypeRef;
    using xo::scm::AType;
    using xo::scm::DVariable;
    using xo::scm::DVarRef;
    using xo::scm::DGlobalSymtab;
    using xo::scm::DConstant;
    using xo::scm::DIfElseExpr;
    using xo::scm::DSequenceExpr;
    using xo::scm::DDefineExpr;
    using xo::scm::DApplyExpr;
    using xo::scm::DLocalSymtab;
    using xo::scm::DLambdaExpr;
    using xo::scm::DTypename;
    using xo::scm::DAtomicType;
    using xo::scm::Metatype;
    using xo::scm::AExpression;
    using xo::scm::DFloat;
    using xo::scm::DInteger;
    using xo::scm::Binding;
    using xo::scm::DUniqueString;
    using xo::scm::StringTable;
    using xo::mm::CollectorTypeRegistry;
    using xo::mm::AAllocator;
    using xo::mm::ACollector;
    using xo::mm::DX1Collector;
    using xo::mm::DArena;
    using xo::mm::AGCObject;
    using xo::mm::X1CollectorConfig;
    using xo::mm::ArenaConfig;
    using xo::print::APrintable;
    using xo::facet::obj;
    using xo::facet::with_facet;
    using xo::map::ArenaHashMapConfig;
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

            /** index of the digits belonging to the ".tseq" at/after @p from,
             *  or npos.  @p n_digit receives their count.
             *
             *  NB the separator is whitespace, not necessarily a SPACE: at a
             *  narrow margin the value breaks onto its own line, so ".tseq" is
             *  followed by "\n" plus indent.  Keying on ".tseq " silently
             *  scrubbed nothing in exactly those cases -- caught because the
             *  broken-layout expectations then failed.
             **/
            std::size_t find_tseq_digits(const std::string & s, std::size_t from,
                                         std::size_t * n_digit) {
                const std::string key = ".tseq";

                for (std::size_t i = s.find(key, from); i != std::string::npos;
                     i = s.find(key, i+1))
                {
                    std::size_t b = i + key.size();

                    while (b < s.size() && ::isspace((unsigned char)s[b]))
                        ++b;

                    std::size_t e = b;

                    while (e < s.size() && ::isdigit((unsigned char)s[e]))
                        ++e;

                    if (e > b) {
                        *n_digit = e - b;
                        return b;
                    }
                }

                return std::string::npos;
            }

            /** replace the digits in a quoted GENERATED type-variable name:
             *  :id "if:12"  ->  :id "if:N".
             *
             *  A third counter, distinct from TypeDescr's :id (scrub_type_id)
             *  and from typeseq (scrub_tseq).  DIfElseExpr::_make_empty() builds
             *  its TypeRef through TypeRef::generate_unique(), which draws on a
             *  process-wide counter -- so these move with how many TypeRefs the
             *  run happened to make before this one, including from unrelated
             *  tests.  The PREFIX is kept: "if:" is a property of the printer
             *  under test, the number is not.
             **/
            std::string scrub_typevar(std::string s) {
                const std::string key = ":id \"";

                for (std::size_t i = s.find(key); i != std::string::npos; i = s.find(key, i+1)) {
                    std::size_t b = i + key.size();

                    /* skip the prefix, up to and including its ':' */
                    while (b < s.size() && s[b] != ':' && s[b] != '"')
                        ++b;

                    if (b >= s.size() || s[b] != ':')
                        continue;   /* no prefix separator -- not a generated name */

                    ++b;

                    std::size_t e = b;

                    while (e < s.size() && ::isdigit((unsigned char)s[e]))
                        ++e;

                    if (e > b)
                        s.replace(b, e - b, "N");
                }

                return s;
            }

            /** replace each ".tseq" value with "N".
             *
             *  Same reasoning as scrub_type_id, different counter: a typeseq is
             *  handed out by xo::reflect::typeseq::id<T>() on first use, and in
             *  practice that is subsystem registration order
             *  (SetupObject2::register_facets etc).  Stable today -- DInteger
             *  is 9 and DFloat 10 on every run -- but it is registration order,
             *  not a property of DConstant, so pinning the digits would make an
             *  unrelated object2 registration break this test.
             *
             *  What the digits actually SAY is checked separately, in
             *  DConstant-tseq-fields below.
             **/
            std::string scrub_tseq(std::string s) {
                std::size_t n_digit = 0;

                for (std::size_t i = find_tseq_digits(s, 0, &n_digit);
                     i != std::string::npos;
                     i = find_tseq_digits(s, i+1, &n_digit))
                {
                    s.replace(i, n_digit, "N");
                }

                return s;
            }

            /** the first ".tseq" value in @p s, as a number; -1 if there is
             *  none.  Used only by DConstant-tseq-fields.
             **/
            long first_tseq(const std::string & s) {
                std::size_t n_digit = 0;
                std::size_t i = find_tseq_digits(s, 0, &n_digit);

                if (i == std::string::npos)
                    return -1;

                return ::strtol(s.c_str() + i, nullptr, 10);
            }

            /** the SECOND ".tseq" value in @p s; -1 if there is none **/
            long second_tseq(const std::string & s) {
                std::size_t n_digit = 0;
                std::size_t i = find_tseq_digits(s, 0, &n_digit);

                if (i == std::string::npos)
                    return -1;

                std::size_t j = find_tseq_digits(s, i+1, &n_digit);

                if (j == std::string::npos)
                    return -1;

                return ::strtol(s.c_str() + j, nullptr, 10);
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
                                 const char * expect_pretty)
                    : kind_{kind}, margin_{margin},
                      expect_pretty_{expect_pretty} {}

                Kind kind_;
                std::uint32_t margin_;
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
                                 "<TypeRef :id \"t:1\" :td null>"),
                /* the struct breaks; fields still fit their own lines, and the
                 * struct-level indent agrees at 2.
                 */
                Testcase_TypeRef(Kind::unresolved, 20,
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
                                 " :metatype atomic>>"),

                /* resolved: :td's value fits on its own line.  identical. */
                Testcase_TypeRef(Kind::both, 80,
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

                /** a DGlobalSymtab holding @p name_v as global variables.
                 *
                 *  @p hint_capacity feeds ArenaHashMapConfig; the rendered
                 *  :var_capacity / :type_capacity come from it (rounded up to a
                 *  power of 2 by DArenaHashMap), so it is a case variable
                 *  rather than a detail.
                 *
                 *  The hash-map superstructure lives outside GC space, hence
                 *  the separate aux arena -- that is DGlobalSymtab::make's own
                 *  mm / aux_mm split, not a testing convenience.
                 **/
                obj<AGCObject,DGlobalSymtab> make_symtab(std::size_t hint_capacity,
                                                         const std::vector<const char *> & name_v) {
                    auto cfg = ArenaHashMapConfig()
                        .with_name("printable_render.symtab")
                        .with_hint_max_capacity(hint_capacity);

                    auto symtab = DGlobalSymtab::make(this->allocator(),
                                                      this->aux_allocator(),
                                                      cfg, cfg);

                    for (const char * name : name_v) {
                        DVariable * var
                            = DVariable::make(this->allocator(),
                                              table_.intern(name),
                                              make_typeref(Kind::resolved));

                        symtab.data()->upsert_variable(this->allocator(), var);
                    }

                    return symtab;
                }

                obj<AAllocator> aux_allocator() { return with_facet<AAllocator>::mkobj(&aux_); }

                /** a DConstant boxing @p x.  Integer and float take different
                 *  paths through DConstant::_lookup_td, and render different
                 *  D-type typeseqs, so both are cases.
                 **/
                /** a DSequenceExpr holding @p n DConstant elements.
                 *
                 *  n == 0 is a case in its own right: an empty DArray is what
                 *  the sequence starts as, and how the nested sequence printer
                 *  renders nothing is worth pinning.
                 **/
                DSequenceExpr * make_sequence(int n) {
                    DSequenceExpr * retval = DSequenceExpr::_make_empty(this->allocator());

                    for (int i = 1; i <= n; ++i) {
                        retval->push_back(this->allocator(),
                                          obj<AExpression>(DConstant::make(this->allocator(),
                                                                           DInteger::box<AGCObject>(this->allocator(), i))));
                    }

                    return retval;
                }

                /** a DIfElseExpr with each branch present or absent.
                 *
                 *  Children are DConstants -- converted printers, so nothing
                 *  here can pin "STUB:" text that would move later.
                 **/
                DIfElseExpr * make_ifelse(bool with_test, bool with_true, bool with_false) {
                    DIfElseExpr * retval = DIfElseExpr::_make_empty(this->allocator());

                    if (with_test)
                        retval->assign_test(obj<AExpression>(DConstant::make(this->allocator(),
                                                                            DInteger::box<AGCObject>(this->allocator(), 1))));
                    if (with_true)
                        retval->assign_when_true(obj<AExpression>(DConstant::make(this->allocator(),
                                                                                  DInteger::box<AGCObject>(this->allocator(), 2))));
                    if (with_false)
                        retval->assign_when_false(obj<AExpression>(DConstant::make(this->allocator(),
                                                                                   DInteger::box<AGCObject>(this->allocator(), 3))));

                    return retval;
                }

                /** a DDefineExpr for @p name, with an initializer iff @p with_rhs.
                 *
                 *  Two independent branches meet here: the OPTIONAL :rhs field,
                 *  and the nested DVariable's own branch on a null name (@p name
                 *  nullptr).  Both are converted printers, so nothing here pins
                 *  "STUB:" text that would move later.
                 *
                 *  make_empty() rather than make(): make() would need an
                 *  obj<AExpression> up front, and the parser's own path is to
                 *  build the skeleton and fill it in.
                 **/
                DDefineExpr * make_define(const char * name, bool with_rhs) {
                    DDefineExpr * retval = DDefineExpr::make_empty(this->allocator());

                    if (name)
                        retval->assign_lhs_name(table_.intern(name));

                    if (with_rhs)
                        retval->assign_rhs(obj<AExpression>(DConstant::make(this->allocator(),
                                                                           DInteger::box<AGCObject>(this->allocator(), 7))));

                    return retval;
                }

                /** a DApplyExpr calling variable "f" with @p n_arg constants.
                 *
                 *  scaffold() + assign_arg() rather than make2(), so the arity
                 *  is a case variable -- the whole point of this printer, whose
                 *  field count is a runtime value.
                 *
                 *  Every argument IS assigned: the printer reaches its children
                 *  through FacetRegistry::variant<APrintable> (not try_variant),
                 *  so an unassigned slot would be an empty obj<> handed to a
                 *  facet lookup that does not tolerate one.  A DApplyExpr with
                 *  holes is a parser-intermediate state; whether it should be
                 *  printable is a separate question from this conversion.
                 *
                 *  fn is a DVariable and the args are DConstants -- both
                 *  converted, so nothing here pins "STUB:" text.
                 **/
                DApplyExpr * make_applyexpr(int n_arg) {
                    /* Kind::unresolved: its TypeRef renders `:id "t:1" :td null`.
                     * Kind::resolved would drag in xo-reflect's TypeDescr
                     * printer -- `<TypeDescr :id 8 :canonical_name double ...>`,
                     * long, and carrying a process-wide counter -- for no gain
                     * here.  This printer's subject is field ARITY.
                     */
                    DVariable * fn_var = this->make_var("f", Kind::unresolved);

                    DApplyExpr * retval
                        = DApplyExpr::scaffold(this->allocator(),
                                               make_typeref(Kind::resolved),
                                               with_facet<AExpression>::mkobj(fn_var),
                                               n_arg);

                    for (int i = 0; i < n_arg; ++i) {
                        retval->assign_arg(i,
                                           obj<AExpression>(DConstant::make(this->allocator(),
                                                                            DInteger::box<AGCObject>(this->allocator(),
                                                                                                     10 + i))));
                    }

                    return retval;
                }

                /** a DLocalSymtab holding @p n_var variables and @p n_type
                 *  type definitions.
                 *
                 *  Two independent dynamic-arity loops, which is what makes
                 *  this printer different from DApplyExpr's one.  Capacity is
                 *  exactly the requested count: append_var/append_type assert
                 *  and no-op past capacity, so an over-tight array would
                 *  silently produce fewer fields than the case names.
                 **/
                DLocalSymtab * make_localsymtab(int n_var, int n_type) {
                    DLocalSymtab * retval
                        = DLocalSymtab::_make_empty(this->allocator(),
                                                    nullptr /*parent*/,
                                                    n_var, n_type);

                    for (int i = 0; i < n_var; ++i) {
                        std::string name = "v" + std::to_string(1 + i);

                        retval->append_var(this->allocator(),
                                           table_.intern(name.c_str()),
                                           make_typeref(Kind::unresolved));
                    }

                    for (int i = 0; i < n_type; ++i) {
                        std::string name = "t" + std::to_string(1 + i);

                        retval->append_type(this->allocator(),
                                            table_.intern(name.c_str()),
                                            DAtomicType::make(this->allocator(),
                                                              Metatype::t_i64()));
                    }

                    return retval;
                }

                /** a DLambdaExpr named @p name, over a symtab of @p n_var
                 *  arguments, with a body iff @p with_body.
                 *
                 *  Legacy's branch is ALL-OR-NOTHING: `name_ && body` gates the
                 *  whole struct, so @p name nullptr and @p with_body false are
                 *  each a case that renders a bare <LambdaExpr> -- not a struct
                 *  with fields dropped.  Both are exercised.
                 *
                 *  The symtab is always real, including for the bare cases: a
                 *  null one would be a different defect from the one under
                 *  test, and visit_gco_children dereferences it.
                 **/
                DLambdaExpr * make_lambda(const char * name, bool with_body, int n_var) {
                    DLocalSymtab * symtab = this->make_localsymtab(n_var, 0 /*n_type*/);

                    obj<AExpression> body;

                    if (with_body) {
                        body = obj<AExpression>(DConstant::make(this->allocator(),
                                                                DInteger::box<AGCObject>(this->allocator(), 5)));
                    }

                    return DLambdaExpr::_make(this->allocator(),
                                              make_typeref(Kind::unresolved),
                                              (name ? table_.intern(name) : nullptr),
                                              symtab,
                                              body);
                }

                /** a DTypename binding @p name to a type, or to NOTHING when
                 *  @p with_type is false.
                 *
                 *  The null-type case is the only one that can render: a
                 *  populated DTypename throws, deliberately.  See
                 *  DTypename-render below.
                 **/
                obj<AGCObject,DTypename> make_typename(const char * name, bool with_type) {
                    obj<AType> type;

                    if (with_type)
                        type = DAtomicType::make(this->allocator(), Metatype::t_i64());

                    return DTypename::make(this->allocator(),
                                           (name ? table_.intern(name) : nullptr),
                                           type);
                }

                template <typename T>
                DConstant * make_constant(T x) {
                    /* if constexpr, not a ternary: the two box<> calls return
                     * different obj<> specializations with no common type.
                     */
                    if constexpr (std::is_integral_v<T>) {
                        obj<AGCObject> value
                            = DInteger::box<AGCObject>(this->allocator(),
                                                       static_cast<long>(x));

                        return DConstant::_make(this->allocator(), value);
                    } else {
                        obj<AGCObject> value
                            = DFloat::box<AGCObject>(this->allocator(),
                                                     static_cast<double>(x));

                        return DConstant::_make(this->allocator(), value);
                    }
                }

                DX1Collector gc_;
                /** non-GC memory for the symbol table's hash maps **/
                DArena aux_ = DArena::map(ArenaConfig{ .name_ = "printable_render.aux",
                                                       .size_ = 256*1024 });
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
                                   const char * expect_pretty)
                    : name_{name}, kind_{kind}, margin_{margin}, label_{label},
                      expect_pretty_{expect_pretty} {}

                /** nullptr -> anonymous variable **/
                const char * name_;
                Kind kind_;
                std::uint32_t margin_;
                /** distinguishes this case's arena **/
                const char * label_;
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
                                   " <TypeRef :id \"t:1\" :td null>>"),

                /* ... at 40 only the OUTER struct breaks, and the nested
                 * TypeRef still fits its line.  Identical, which is the useful
                 * part: the two stacks agree wherever nothing is forced.
                 */
                Testcase_DVariable("myvar", Kind::unresolved, 40, "unres40",
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
                                 const char * expect_pretty)
                    : name_{name}, path_{path}, link_{link}, margin_{margin},
                      label_{label},
                      expect_pretty_{expect_pretty} {}

                const char * name_;
                Binding path_;
                std::int32_t link_;
                std::uint32_t margin_;
                const char * label_;
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
                                 "<DVarRef :name myvar :path {path:0:3}>"),

                /* outer struct breaks; both values still fit their lines */
                Testcase_DVarRef("myvar", Binding::local(3), 0, 30, "local0.30",
                                 "<DVarRef\n"
                                 "  :name myvar\n"
                                 "  :path {path:0:3}>"),

                /* margin 12: BOTH values break, so the known field-value column
                 * divergence (legacy 4, ppsink 3) shows up twice in one render.
                 */
                Testcase_DVarRef("myvar", Binding::local(3), 0, 12, "local0.12",
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
                                 "<DVarRef :name myvar :path {path:2:3}>"),

                /* a global binding prints its own way ("{path:global:7}",
                 * Binding::print) and ignores the link -- Binding::relative
                 * returns the definition unchanged for globals.
                 */
                Testcase_DVarRef("g", Binding::global(7), 0, 200, "global.200",
                                 "<DVarRef :name g :path {path:global:7}>"),

                /* margin 20: :name fits, :path does not -- the divergence on
                 * one field with the other left alone.
                 */
                Testcase_DVarRef("g", Binding::global(7), 0, 20, "global.20",
                                 "<DVarRef\n"
                                 "  :name g\n"
                                 "  :path\n"
                                 "   {path:global:7}>"),
            };

            /** DGlobalSymtab's four fields are all std::uint32_t
             *  (DGlobalSymtab.hpp), so this is the first printer whose every
             *  value goes through the WIDENED integer Prettifier rather than
             *  the operator<< fallback -- see
             *  .xo-backlog/xo-ppsink/issues/09-scalar-prettifiers.md, which was
             *  done first precisely so these expectations pin a Prettifier.
             **/
            struct Testcase_DGlobalSymtab {
                Testcase_DGlobalSymtab(std::size_t hint_capacity,
                                       std::vector<const char *> name_v,
                                       std::uint32_t margin,
                                       const char * label,
                                       const char * expect_pretty)
                    : hint_capacity_{hint_capacity}, name_v_{std::move(name_v)},
                      margin_{margin}, label_{label},
                      expect_pretty_{expect_pretty} {}

                std::size_t hint_capacity_;
                std::vector<const char *> name_v_;
                std::uint32_t margin_;
                const char * label_;
                /** OBSERVED via pretty; outlives phase E **/
                std::string expect_pretty_;
            };

            static std::vector<Testcase_DGlobalSymtab> s_symtab_v = {
                /* an empty symtab still reports a capacity: the hash map is
                 * sized at construction.  16, not the hint of 8 --
                 * DArenaHashMap rounds up.
                 */
                Testcase_DGlobalSymtab(8, {}, 200, "empty.200",
                                       "<DGlobalSymtab :nvar 0 :var_capacity 16"
                                       " :ntype 0 :type_capacity 16>"),

                /* struct breaks, every field fits its own line.  Identical --
                 * four scalar fields have no nested structure to disagree over,
                 * which is what makes this printer the safe one to do first.
                 */
                Testcase_DGlobalSymtab(8, {}, 40, "empty.40",
                                       "<DGlobalSymtab\n"
                                       "  :nvar 0\n"
                                       "  :var_capacity 16\n"
                                       "  :ntype 0\n"
                                       "  :type_capacity 16>"),

                /* margin 14: only the two LONG field names force their values
                 * onto the next line, so the known column divergence (legacy 4,
                 * ppsink 3) appears while :nvar and :ntype stay put.
                 */
                Testcase_DGlobalSymtab(8, {}, 14, "empty.14",
                                       "<DGlobalSymtab\n"
                                       "  :nvar 0\n"
                                       "  :var_capacity\n"
                                       "   16\n"
                                       "  :ntype 0\n"
                                       "  :type_capacity\n"
                                       "   16>"),

                /* three upserted variables: :nvar tracks them, :ntype does not.
                 * Pins that the printer reads the two arrays separately rather
                 * than one count twice.
                 */
                Testcase_DGlobalSymtab(8, {"a", "b", "c"}, 200, "three.200",
                                       "<DGlobalSymtab :nvar 3 :var_capacity 16"
                                       " :ntype 0 :type_capacity 16>"),

                Testcase_DGlobalSymtab(8, {"a", "b", "c"}, 14, "three.14",
                                       "<DGlobalSymtab\n"
                                       "  :nvar 3\n"
                                       "  :var_capacity\n"
                                       "   16\n"
                                       "  :ntype 0\n"
                                       "  :type_capacity\n"
                                       "   16>"),

                /* a wider hint moves BOTH capacities, so they are read from the
                 * maps rather than being a constant that happened to match.
                 */
                Testcase_DGlobalSymtab(64, {"a"}, 200, "wide.200",
                                       "<DGlobalSymtab :nvar 1 :var_capacity 64"
                                       " :ntype 0 :type_capacity 64>"),
            };



            /** DSequenceExpr is one field wrapping a DArray -- so it is the
             *  first expression2 printer nesting a SEQUENCE rather than a
             *  struct, and the DArray framing divergence settled in
             *  xo-object2's phase C reappears here one level down.
             **/
            struct Testcase_DSequenceExpr {
                Testcase_DSequenceExpr(int n_elt,
                                       std::uint32_t margin,
                                       const char * label,
                                       const char * expect_pretty)
                    : n_elt_{n_elt}, margin_{margin}, label_{label},
                      expect_pretty_{expect_pretty} {}

                int n_elt_;
                std::uint32_t margin_;
                const char * label_;
                /** OBSERVED via pretty; outlives phase E **/
                std::string expect_pretty_;
            };

            static std::vector<Testcase_DSequenceExpr> s_sequence_v = {
                /* empty sequence: "[]", and identical at EVERY margin -- an
                 * empty DArray has no break points to offer, so this case never
                 * diverges however narrow.
                 */
                Testcase_DSequenceExpr(0, 200, "seq0.200",
                                       "<DSequenceExpr :expr_v []>"),

                Testcase_DSequenceExpr(0, 30, "seq0.30",
                                       "<DSequenceExpr :expr_v []>"),

                /* one element, flat: identical */
                Testcase_DSequenceExpr(1, 200, "seq1.200",
                                       "<DSequenceExpr :expr_v [<DConstant :value_.tseq N :value.tseq N :value 1>]>"),

                /* REVIEWED DIVERGENCE, and it is DArray's, not this printer's
                 * -- already settled in xo-object2's phase C and reappearing
                 * because :expr_v nests a DArray:
                 *
                 *   legacy   "[ <DConstant"   -- a space after '[', elements
                 *                                aligned at column 6
                 *   ppsink   "[<DConstant"    -- no space, elements at column 3
                 *
                 * Stacked on top of the field-value column divergence, so this
                 * case shows both at once.
                 */
                Testcase_DSequenceExpr(1, 30, "seq1.30",
                                       "<DSequenceExpr\n"
                                       "  :expr_v\n"
                                       "   [<DConstant\n"
                                       "     :value_.tseq N\n"
                                       "     :value.tseq N\n"
                                       "     :value 1>]>"),

                /* three elements at margin 60: the elements each fit a line, so
                 * this isolates the ELEMENT ALIGNMENT half of the divergence --
                 * legacy continues at column 6, ppsink at column 3 -- without
                 * the elements themselves breaking.
                 */
                Testcase_DSequenceExpr(3, 60, "seq3.60",
                                       "<DSequenceExpr\n"
                                       "  :expr_v\n"
                                       "   [<DConstant :value_.tseq N :value.tseq N :value 1>\n"
                                       "   <DConstant :value_.tseq N :value.tseq N :value 2>\n"
                                       "   <DConstant :value_.tseq N :value.tseq N :value 3>]>"),

                /* three elements flat: identical, so the divergence is purely
                 * about where breaks land, not about which tokens are emitted.
                 */
                Testcase_DSequenceExpr(3, 200, "seq3.200",
                                       "<DSequenceExpr :expr_v [<DConstant :value_.tseq N :value.tseq N :value 1> <DConstant :value_.tseq N :value.tseq N :value 2> <DConstant :value_.tseq N :value.tseq N :value 3>]>"),
            };

            /** DIfElseExpr is the first printer with OPTIONAL fields: legacy
             *  uses refrtag's three-argument form and ppsink field()'s third
             *  argument, and an absent branch drops the field AND its separator
             *  rather than rendering an empty value.
             *
             *  Children are DConstants (converted), so nothing here pins
             *  "STUB:" text that would move when a sibling printer lands.
             **/
            struct Testcase_DIfElseExpr {
                Testcase_DIfElseExpr(bool with_test, bool with_true, bool with_false,
                                     std::uint32_t margin,
                                     const char * label,
                                     const char * expect_pretty)
                    : with_test_{with_test}, with_true_{with_true},
                      with_false_{with_false}, margin_{margin}, label_{label},
                      expect_pretty_{expect_pretty} {}

                bool with_test_;
                bool with_true_;
                bool with_false_;
                std::uint32_t margin_;
                const char * label_;
                /** OBSERVED via pretty; outlives phase E **/
                std::string expect_pretty_;
            };

            static std::vector<Testcase_DIfElseExpr> s_ifelse_v = {
                /* THE case this printer was worth doing for: all three branches
                 * absent, so three of four fields vanish and only :typeref is
                 * rendered.  Both stacks agree -- no ":test" with an empty
                 * value, no stray separator.
                 */
                Testcase_DIfElseExpr(false, false, false, 200, "none.200",
                                     "<DIfElseExpr :typeref <TypeRef :id \"if:N\" :td null>>"),

                /* the same, broken: the sole field still breaks normally, and
                 * the nested TypeRef shows the usual column divergence.
                 */
                Testcase_DIfElseExpr(false, false, false, 30, "none.30",
                                     "<DIfElseExpr\n"
                                     "  :typeref\n"
                                     "   <TypeRef\n"
                                     "    :id \"if:N\"\n"
                                     "    :td null>>"),

                /* one branch present: :test appears, :when_true / :when_false
                 * stay absent.  Pins that presence is PER FIELD, not all-or-
                 * nothing.
                 */
                Testcase_DIfElseExpr(true, false, false, 200, "test-only.200",
                                     "<DIfElseExpr :typeref <TypeRef :id \"if:N\" :td null> :test <DConstant :value_.tseq N :value.tseq N :value 1>>"),

                /* two present, and at margin 60 the second child's value is
                 * pushed to its own line -- the column divergence again, with
                 * an absent field still cleanly missing.
                 */
                Testcase_DIfElseExpr(true, true, false, 60, "no-else.60",
                                     "<DIfElseExpr\n"
                                     "  :typeref <TypeRef :id \"if:N\" :td null>\n"
                                     "  :test <DConstant :value_.tseq N :value.tseq N :value 1>\n"
                                     "  :when_true\n"
                                     "   <DConstant :value_.tseq N :value.tseq N :value 2>>"),

                /* all four fields.  NB this breaks even at margin 200 -- the
                 * flat form is 190+ characters -- so there is no all-on-one-line
                 * case for this printer, and the two stacks agree exactly here.
                 */
                Testcase_DIfElseExpr(true, true, true, 200, "all.200",
                                     "<DIfElseExpr\n"
                                     "  :typeref <TypeRef :id \"if:N\" :td null>\n"
                                     "  :test <DConstant :value_.tseq N :value.tseq N :value 1>\n"
                                     "  :when_true <DConstant :value_.tseq N :value.tseq N :value 2>\n"
                                     "  :when_false <DConstant :value_.tseq N :value.tseq N :value 3>>"),

                /* margin 30: every child breaks, so the field-value column
                 * divergence (legacy 4, ppsink 3) appears at both levels for
                 * all four fields at once -- the widest instance so far.
                 */
                Testcase_DIfElseExpr(true, true, true, 30, "all.30",
                                     "<DIfElseExpr\n"
                                     "  :typeref\n"
                                     "   <TypeRef\n"
                                     "    :id \"if:N\"\n"
                                     "    :td null>\n"
                                     "  :test\n"
                                     "   <DConstant\n"
                                     "    :value_.tseq N\n"
                                     "    :value.tseq N\n"
                                     "    :value 1>\n"
                                     "  :when_true\n"
                                     "   <DConstant\n"
                                     "    :value_.tseq N\n"
                                     "    :value.tseq N\n"
                                     "    :value 2>\n"
                                     "  :when_false\n"
                                     "   <DConstant\n"
                                     "    :value_.tseq N\n"
                                     "    :value.tseq N\n"
                                     "    :value 3>>"),
            };

            /** DConstant's :value nests an object2 leaf (DInteger / DFloat),
             *  both already converted -- so this is the first expression2
             *  printer nesting a printer from ANOTHER subsystem.
             *
             *  is_int_ picks the boxed type; the two .tseq fields are scrubbed
             *  (see scrub_tseq), so what varies visibly is the :value leaf and
             *  the layout.
             **/
            struct Testcase_DConstant {
                Testcase_DConstant(bool is_int,
                                   std::uint32_t margin,
                                   const char * label,
                                   const char * expect_pretty)
                    : is_int_{is_int}, margin_{margin}, label_{label},
                      expect_pretty_{expect_pretty} {}

                bool is_int_;
                std::uint32_t margin_;
                const char * label_;
                /** OBSERVED via pretty; outlives phase E **/
                std::string expect_pretty_;
            };

            static std::vector<Testcase_DConstant> s_constant_v = {
                /* flat.  :value 42 comes from DInteger's own converted printer,
                 * reached through the APrintable facet variant -- so this pins
                 * cross-subsystem nesting, not just DConstant's frame.
                 */
                Testcase_DConstant(true, 200, "int.200",
                                   "<DConstant :value_.tseq N :value.tseq N :value 42>"),

                /* struct breaks, all three values still fit their lines */
                Testcase_DConstant(true, 44, "int.44",
                                   "<DConstant\n"
                                   "  :value_.tseq N\n"
                                   "  :value.tseq N\n"
                                   "  :value 42>"),

                /* margin 14: the two long field names push their values down --
                 * the known column divergence (legacy 4, ppsink 3) -- while
                 * :value 42 stays put.  Same shape DGlobalSymtab showed.
                 */
                Testcase_DConstant(true, 14, "int.14",
                                   "<DConstant\n"
                                   "  :value_.tseq\n"
                                   "   N\n"
                                   "  :value.tseq\n"
                                   "   N\n"
                                   "  :value 42>"),

                /* a float constant: DFloat's Prettifier renders 2.5, and the
                 * boxed type is different (which scrub_tseq hides here and
                 * DConstant-tseq-fields checks instead).
                 */
                Testcase_DConstant(false, 200, "flt.200",
                                   "<DConstant :value_.tseq N :value.tseq N :value 2.5>"),

                Testcase_DConstant(false, 14, "flt.14",
                                   "<DConstant\n"
                                   "  :value_.tseq\n"
                                   "   N\n"
                                   "  :value.tseq\n"
                                   "   N\n"
                                   "  :value 2.5>"),
            };

            /** DDefineExpr has one optional field (:rhs) and one that is always
             *  present (:lhs).  Where DIfElseExpr's legacy body used refrtag's
             *  three-argument form, this one DUPLICATES the whole
             *  pretty_struct() call in an if/else -- see the "cond() doesn't
             *  resolve the way we want here" comment in DDefineExpr.cpp.  The
             *  ppsink body is a single call with field()'s third argument, so
             *  these cases are the evidence that the two spellings agree.
             *
             *  @p name nullptr exercises the nested DVariable's own null-name
             *  branch at the same time.
             **/
            struct Testcase_DDefineExpr {
                Testcase_DDefineExpr(const char * name, bool with_rhs,
                                     std::uint32_t margin,
                                     const char * label,
                                     const char * expect_pretty)
                    : name_{name}, with_rhs_{with_rhs}, margin_{margin},
                      label_{label},
                      expect_pretty_{expect_pretty} {}

                /** nullptr -> lhs variable is anonymous **/
                const char * name_;
                bool with_rhs_;
                std::uint32_t margin_;
                const char * label_;
                /** OBSERVED via pretty; outlives phase E **/
                std::string expect_pretty_;
            };

            static std::vector<Testcase_DDefineExpr> s_define_v = {
                /* no initializer: :rhs and its separator are gone entirely.
                 * Both stacks agree -- no ":rhs" with an empty value.
                 */
                Testcase_DDefineExpr("x", false, 200, "noinit.200",
                                     "<DDefineExpr :lhs <DVariable :name \"x\" :typeref <TypeRef :id \"\" :td null>>>"),

                /* the same, fully broken.  Three levels of nesting, so the
                 * field-value column divergence (legacy indent+2, ppsink
                 * indent+1) compounds: 4 vs 3 at :lhs, 8 vs 5 at :typeref.
                 */
                Testcase_DDefineExpr("x", false, 30, "noinit.30",
                                     "<DDefineExpr\n"
                                     "  :lhs\n"
                                     "   <DVariable\n"
                                     "    :name \"x\"\n"
                                     "    :typeref\n"
                                     "     <TypeRef\n"
                                     "      :id \"\"\n"
                                     "      :td null>>>"),

                /* with an initializer: :rhs appears, flat, identical */
                Testcase_DDefineExpr("x", true, 200, "init.200",
                                     "<DDefineExpr :lhs <DVariable :name \"x\" :typeref <TypeRef :id \"\" :td null>> :rhs <DConstant :value_.tseq N :value.tseq N :value 7>>"),

                /* margin 60 -- THE case worth having.  Here the column
                 * divergence changes WHAT IS EMITTED, not just where: ppsink's
                 * :lhs value starts one column earlier, which leaves the nested
                 * DVariable enough room to stay on one line, where legacy has
                 * to break it into four.  Elsewhere the two stacks differ only
                 * in leading whitespace; this pins that the difference can
                 * cascade into a different line structure.
                 */
                Testcase_DDefineExpr("x", true, 60, "init.60",
                                     "<DDefineExpr\n"
                                     "  :lhs\n"
                                     "   <DVariable :name \"x\" :typeref <TypeRef :id \"\" :td null>>\n"
                                     "  :rhs <DConstant :value_.tseq N :value.tseq N :value 7>>"),

                /* margin 30, both fields present: every level breaks */
                Testcase_DDefineExpr("x", true, 30, "init.30",
                                     "<DDefineExpr\n"
                                     "  :lhs\n"
                                     "   <DVariable\n"
                                     "    :name \"x\"\n"
                                     "    :typeref\n"
                                     "     <TypeRef\n"
                                     "      :id \"\"\n"
                                     "      :td null>>\n"
                                     "  :rhs\n"
                                     "   <DConstant\n"
                                     "    :value_.tseq N\n"
                                     "    :value.tseq N\n"
                                     "    :value 7>>"),

                /* anonymous lhs: DVariable renders :name "" rather than
                 * dropping the field -- an empty VALUE, not an absent field.
                 * The contrast with :rhs above is the point.
                 */
                Testcase_DDefineExpr(nullptr, true, 200, "anon.200",
                                     "<DDefineExpr :lhs <DVariable :name \"\" :typeref <TypeRef :id \"\" :td null>> :rhs <DConstant :value_.tseq N :value.tseq N :value 7>>"),
            };

            /** DApplyExpr's field count is n_args_+1, a RUNTIME value, so this
             *  is the first printer in the cluster built with
             *  PpSink::struct_open() rather than pretty_struct().  Field names
             *  after :fn are generated -- concat("arg", 1+i), 1-based.
             *
             *  fn is a DVariable and the args are DConstants, both converted.
             **/
            struct Testcase_DApplyExpr {
                Testcase_DApplyExpr(int n_arg, std::uint32_t margin,
                                    const char * label,
                                    const char * expect_pretty)
                    : n_arg_{n_arg}, margin_{margin}, label_{label},
                      expect_pretty_{expect_pretty} {}

                int n_arg_;
                std::uint32_t margin_;
                const char * label_;
                /** OBSERVED via pretty; outlives phase E **/
                std::string expect_pretty_;
            };

            static std::vector<Testcase_DApplyExpr> s_apply_v = {
                /* zero args -- only :fn.  THE <DVariable :name \"f\" :typeref <TypeRef :id \"t:N\" :td null>>-FORM DIVERGENCE: legacy
                 * renders "<ApplyExpr:fn ..." with no space, because its
                 * hand-rolled print_upto() path writes the struct name and then
                 * each refrtag with no separator between them.  ppsink emits
                 * one, as every pretty_struct-based printer already did.
                 * Legacy is wrong here and this is not a regression; see the
                 * ticket.
                 */
                Testcase_DApplyExpr(0, 200, "a0.200",
                                    "<ApplyExpr :fn <DVariable :name \"f\" :typeref <TypeRef :id \"t:N\" :td null>>>"),

                /* broken: the separator question disappears -- a field on its
                 * own line needs no leading space -- so the two stacks differ
                 * only by the usual field-value column (legacy +2, ppsink +1),
                 * compounding over three levels.
                 */
                Testcase_DApplyExpr(0, 30, "a0.30",
                                    "<ApplyExpr\n"
                                    "  :fn\n"
                                    "   <DVariable\n"
                                    "    :name \"f\"\n"
                                    "    :typeref\n"
                                    "     <TypeRef\n"
                                    "      :id \"t:N\"\n"
                                    "      :td null>>>"),

                /* one arg, flat: TWO missing separators in legacy, before :fn
                 * and before :arg1.  Pins that the defect is per field.
                 */
                Testcase_DApplyExpr(1, 200, "a1.200",
                                    "<ApplyExpr :fn <DVariable :name \"f\" :typeref <TypeRef :id \"t:N\" :td null>> :arg1 <DConstant :value_.tseq N :value.tseq N :value 10>>"),

                Testcase_DApplyExpr(1, 40, "a1.40",
                                    "<ApplyExpr\n"
                                    "  :fn\n"
                                    "   <DVariable\n"
                                    "    :name \"f\"\n"
                                    "    :typeref\n"
                                    "     <TypeRef :id \"t:N\" :td null>>\n"
                                    "  :arg1\n"
                                    "   <DConstant\n"
                                    "    :value_.tseq N\n"
                                    "    :value.tseq N\n"
                                    "    :value 10>>"),

                /* three args at margin 200: too wide to fit, so it breaks and
                 * every field fits its own line -- the two stacks agree
                 * EXACTLY.  The generated names arg1/arg2/arg3 are the
                 * assertion that matters here.
                 */
                Testcase_DApplyExpr(3, 200, "a3.200",
                                    "<ApplyExpr\n"
                                    "  :fn <DVariable :name \"f\" :typeref <TypeRef :id \"t:N\" :td null>>\n"
                                    "  :arg1 <DConstant :value_.tseq N :value.tseq N :value 10>\n"
                                    "  :arg2 <DConstant :value_.tseq N :value.tseq N :value 11>\n"
                                    "  :arg3 <DConstant :value_.tseq N :value.tseq N :value 12>>"),

                /* margin 60: :fn breaks, the args still fit -- so one field
                 * diverges and three do not, in one rendering.
                 */
                Testcase_DApplyExpr(3, 60, "a3.60",
                                    "<ApplyExpr\n"
                                    "  :fn\n"
                                    "   <DVariable\n"
                                    "    :name \"f\"\n"
                                    "    :typeref <TypeRef :id \"t:N\" :td null>>\n"
                                    "  :arg1 <DConstant :value_.tseq N :value.tseq N :value 10>\n"
                                    "  :arg2 <DConstant :value_.tseq N :value.tseq N :value 11>\n"
                                    "  :arg3 <DConstant :value_.tseq N :value.tseq N :value 12>>"),

                /* margin 30: everything breaks, at every level */
                Testcase_DApplyExpr(3, 30, "a3.30",
                                    "<ApplyExpr\n"
                                    "  :fn\n"
                                    "   <DVariable\n"
                                    "    :name \"f\"\n"
                                    "    :typeref\n"
                                    "     <TypeRef\n"
                                    "      :id \"t:N\"\n"
                                    "      :td null>>\n"
                                    "  :arg1\n"
                                    "   <DConstant\n"
                                    "    :value_.tseq N\n"
                                    "    :value.tseq N\n"
                                    "    :value 10>\n"
                                    "  :arg2\n"
                                    "   <DConstant\n"
                                    "    :value_.tseq N\n"
                                    "    :value.tseq N\n"
                                    "    :value 11>\n"
                                    "  :arg3\n"
                                    "   <DConstant\n"
                                    "    :value_.tseq N\n"
                                    "    :value.tseq N\n"
                                    "    :value 12>>"),
            };
            /** DLocalSymtab has TWO dynamic-arity loops -- vars_ then types_ --
             *  bracketing two scalar fields, :nvars and :ntypes.  Index names
             *  are generated as "[i]" and COLLIDE between the loops: a symtab
             *  with both renders :[0] twice.  Legacy did that too (snprintf
             *  "[%u]" in each loop) and the conversion reproduces it.
             *
             *  Only VAR-ONLY cases are here.  A non-empty types_ cannot be
             *  pinned on either side: legacy THROWS (see
             *  DLocalSymtab-types-throws below), and ppsink renders
             *  "STUB:DTypename", which moves when DTypename converts.
             *
             *  Unlike DApplyExpr, legacy loses no separator here -- it builds
             *  fields with xrefrtag/newline_pretty_tag rather than writing the
             *  struct name and refrtags back to back.  So the ONLY divergence
             *  is the usual field-value column: legacy indent+2, ppsink
             *  indent+1, compounding per level.
             **/
            struct Testcase_DLocalSymtab {
                Testcase_DLocalSymtab(int n_var, std::uint32_t margin,
                                      const char * label,
                                      const char * expect_pretty)
                    : n_var_{n_var}, margin_{margin}, label_{label},
                      expect_pretty_{expect_pretty} {}

                int n_var_;
                std::uint32_t margin_;
                const char * label_;
                /** OBSERVED via pretty; outlives phase E **/
                std::string expect_pretty_;
            };

            static std::vector<Testcase_DLocalSymtab> s_localsymtab_v = {
                /* empty: both scalar fields and neither loop.  The two stacks
                 * agree exactly -- nothing nests, so no field-value column
                 * question arises.
                 */
                Testcase_DLocalSymtab(0, 200, "s0.200",
                                      "<LocalSymtab :nvars 0 :ntypes 0>"),

                Testcase_DLocalSymtab(0, 30, "s0.30",
                                      "<LocalSymtab\n"
                                      "  :nvars 0\n"
                                      "  :ntypes 0>"),

                /* one var, flat.  :ntypes 0 still appears AFTER the loop --
                 * an empty loop drops its fields but not the count.
                 */
                Testcase_DLocalSymtab(1, 200, "s1.200",
                                      "<LocalSymtab :nvars 1 :[0] <DVariable :name \"v1\" :typeref <TypeRef :id \"t:N\" :td null>> :ntypes 0>"),

                /* margin 60: the symtab and the DVariable break, the TypeRef
                 * does not -- one level of divergence, not three.
                 */
                Testcase_DLocalSymtab(1, 60, "s1.60",
                                      "<LocalSymtab\n"
                                      "  :nvars 1\n"
                                      "  :[0]\n"
                                      "   <DVariable\n"
                                      "    :name \"v1\"\n"
                                      "    :typeref <TypeRef :id \"t:N\" :td null>>\n"
                                      "  :ntypes 0>"),

                /* margin 30: everything breaks, so the +2 / +1 column gap
                 * compounds over three levels -- 4 vs 3, 6 vs 4, 8 vs 5.
                 */
                Testcase_DLocalSymtab(1, 30, "s1.30",
                                      "<LocalSymtab\n"
                                      "  :nvars 1\n"
                                      "  :[0]\n"
                                      "   <DVariable\n"
                                      "    :name \"v1\"\n"
                                      "    :typeref\n"
                                      "     <TypeRef\n"
                                      "      :id \"t:N\"\n"
                                      "      :td null>>\n"
                                      "  :ntypes 0>"),

                /* two vars: the generated names ARE the assertion -- [0] then
                 * [1], in append order, with v1/v2 following them.
                 */
                Testcase_DLocalSymtab(2, 200, "s2.200",
                                      "<LocalSymtab :nvars 2 :[0] <DVariable :name \"v1\" :typeref <TypeRef :id \"t:N\" :td null>> :[1] <DVariable :name \"v2\" :typeref <TypeRef :id \"t:N\" :td null>> :ntypes 0>"),

                Testcase_DLocalSymtab(2, 30, "s2.30",
                                      "<LocalSymtab\n"
                                      "  :nvars 2\n"
                                      "  :[0]\n"
                                      "   <DVariable\n"
                                      "    :name \"v1\"\n"
                                      "    :typeref\n"
                                      "     <TypeRef\n"
                                      "      :id \"t:N\"\n"
                                      "      :td null>>\n"
                                      "  :[1]\n"
                                      "   <DVariable\n"
                                      "    :name \"v2\"\n"
                                      "    :typeref\n"
                                      "     <TypeRef\n"
                                      "      :id \"t:N\"\n"
                                      "      :td null>>\n"
                                      "  :ntypes 0>"),
            };

            /** DLambdaExpr's branch is ALL-OR-NOTHING, unlike DDefineExpr's
             *  per-field one: `name_ && body` gates the entire struct, so an
             *  incomplete lambda renders `<LambdaExpr>` rather than a struct
             *  with fields omitted.  Both halves of that condition are cases.
             *
             *  Its :local_symtab field nests DLocalSymtab, converted just
             *  before it -- which is why this printer was taken after that one
             *  rather than before: nothing here pins "STUB:" text.
             **/
            struct Testcase_DLambdaExpr {
                Testcase_DLambdaExpr(const char * name, bool with_body, int n_var,
                                     std::uint32_t margin, const char * label,
                                     const char * expect_pretty)
                    : name_{name}, with_body_{with_body}, n_var_{n_var},
                      margin_{margin}, label_{label},
                      expect_pretty_{expect_pretty} {}

                const char * name_;
                bool with_body_;
                int n_var_;
                std::uint32_t margin_;
                const char * label_;
                /** OBSERVED via pretty; outlives phase E **/
                std::string expect_pretty_;
            };

            static std::vector<Testcase_DLambdaExpr> s_lambda_v = {
                /* fits on one line: the two stacks agree, and nothing nests
                 * deeply enough for the field-value column to matter.
                 */
                Testcase_DLambdaExpr("f", true, 0, 200, "L0.200",
                                      "<LambdaExpr :tref <TypeRef :id \"t:N\" :td null> :name \"f\" :local_symtab <LocalSymtab :nvars 0 :ntypes 0> :body <DConstant :value_.tseq N :value.tseq N :value 5>>"),

                Testcase_DLambdaExpr("f", true, 0, 60, "L0.60",
                                      "<LambdaExpr\n"
                                      "  :tref <TypeRef :id \"t:N\" :td null>\n"
                                      "  :name \"f\"\n"
                                      "  :local_symtab <LocalSymtab :nvars 0 :ntypes 0>\n"
                                      "  :body <DConstant :value_.tseq N :value.tseq N :value 5>>"),

                Testcase_DLambdaExpr("f", true, 0, 30, "L0.30",
                                      "<LambdaExpr\n"
                                      "  :tref\n"
                                      "   <TypeRef\n"
                                      "    :id \"t:N\"\n"
                                      "    :td null>\n"
                                      "  :name \"f\"\n"
                                      "  :local_symtab\n"
                                      "   <LocalSymtab\n"
                                      "    :nvars 0\n"
                                      "    :ntypes 0>\n"
                                      "  :body\n"
                                      "   <DConstant\n"
                                      "    :value_.tseq N\n"
                                      "    :value.tseq N\n"
                                      "    :value 5>>"),

                /* one argument, margin 200.  The :local_symtab value is now wide
                 * enough that the LAMBDA breaks while the symtab inside it does
                 * not -- so every field gets its own line and the two stacks
                 * agree EXACTLY, isolating field order and names from layout.
                 */
                Testcase_DLambdaExpr("f", true, 1, 200, "L1.200",
                                      "<LambdaExpr\n"
                                      "  :tref <TypeRef :id \"t:N\" :td null>\n"
                                      "  :name \"f\"\n"
                                      "  :local_symtab <LocalSymtab :nvars 1 :[0] <DVariable :name \"v1\" :typeref <TypeRef :id \"t:N\" :td null>> :ntypes 0>\n"
                                      "  :body <DConstant :value_.tseq N :value.tseq N :value 5>>"),

                /* the mixed case: the symtab breaks, its one variable does not.
                 * Exactly one level of the +2/+1 column gap is visible.
                 */
                Testcase_DLambdaExpr("f", true, 1, 80, "L1.80",
                                      "<LambdaExpr\n"
                                      "  :tref <TypeRef :id \"t:N\" :td null>\n"
                                      "  :name \"f\"\n"
                                      "  :local_symtab\n"
                                      "   <LocalSymtab\n"
                                      "    :nvars 1\n"
                                      "    :[0] <DVariable :name \"v1\" :typeref <TypeRef :id \"t:N\" :td null>>\n"
                                      "    :ntypes 0>\n"
                                      "  :body <DConstant :value_.tseq N :value.tseq N :value 5>>"),

                /* everything breaks, four levels deep -- the deepest nesting in
                 * this fixture, and where the column gap compounds most.
                 */
                Testcase_DLambdaExpr("f", true, 1, 30, "L1.30",
                                      "<LambdaExpr\n"
                                      "  :tref\n"
                                      "   <TypeRef\n"
                                      "    :id \"t:N\"\n"
                                      "    :td null>\n"
                                      "  :name \"f\"\n"
                                      "  :local_symtab\n"
                                      "   <LocalSymtab\n"
                                      "    :nvars 1\n"
                                      "    :[0]\n"
                                      "     <DVariable\n"
                                      "      :name \"v1\"\n"
                                      "      :typeref\n"
                                      "       <TypeRef\n"
                                      "        :id \"t:N\"\n"
                                      "        :td null>>\n"
                                      "    :ntypes 0>\n"
                                      "  :body\n"
                                      "   <DConstant\n"
                                      "    :value_.tseq N\n"
                                      "    :value.tseq N\n"
                                      "    :value 5>>"),

                Testcase_DLambdaExpr("f", true, 2, 200, "L2.200",
                                      "<LambdaExpr\n"
                                      "  :tref <TypeRef :id \"t:N\" :td null>\n"
                                      "  :name \"f\"\n"
                                      "  :local_symtab <LocalSymtab :nvars 2 :[0] <DVariable :name \"v1\" :typeref <TypeRef :id \"t:N\" :td null>> :[1] <DVariable :name \"v2\" :typeref <TypeRef :id \"t:N\" :td null>> :ntypes 0>\n"
                                      "  :body <DConstant :value_.tseq N :value.tseq N :value 5>>"),

                /* name_ null: the WHOLE struct collapses to a bare <LambdaExpr>.
                 * Not "fields dropped" -- the body and symtab are present and
                 * still render nothing.  That is legacy's branch, reproduced.
                 */
                Testcase_DLambdaExpr(nullptr, true, 1, 200, "Lanon.200",
                                      "<LambdaExpr>"),

                /* and it is margin-invariant, having no break points at all */
                Testcase_DLambdaExpr(nullptr, true, 1, 30, "Lanon.30",
                                      "<LambdaExpr>"),

                /* body absent, name present: the same collapse from the other
                 * half of the condition.
                 */
                Testcase_DLambdaExpr("f", false, 1, 200, "Lnobody.200",
                                      "<LambdaExpr>"),

                /* margin 8 against 13 characters: the degenerate form does not
                 * break even when it cannot fit, because it has nowhere to.
                 */
                Testcase_DLambdaExpr("f", false, 1, 8, "Lnobody.8",
                                      "<LambdaExpr>"),
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
                    std::string pretty = scrub_type_id(render_pretty(tr, tc.margin_));

                    log && log(xtag("i_tc", i_tc), xtag("margin", tc.margin_), xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
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
                    std::string pretty = scrub_type_id(render_pretty(p, tc.margin_));

                    log && log(xtag("i_tc", i_tc), xtag("margin", tc.margin_), xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
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
                    std::string pretty = render_pretty(p, tc.margin_);

                    log && log(xtag("i_tc", i_tc), xtag("margin", tc.margin_), xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
                }
            }
        }

        /** the one case that CANNOT be pinned against both protocols.
         *
         *  DVarRef::pretty (and pretty_deprecated before it) does std::string_view(*(this->name()))
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

        TEST_CASE("DGlobalSymtab-render", "[printable][DGlobalSymtab]")
        {
            REQUIRE(s_init.evidence());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "DGlobalSymtab-render"));

                for (std::size_t i_tc = 0, n_tc = s_symtab_v.size(); i_tc < n_tc; ++i_tc) {
                    const auto & tc = s_symtab_v[i_tc];

                    VarFixture fx(tc.label_);

                    auto symtab = fx.make_symtab(tc.hint_capacity_, tc.name_v_);

                    auto p = with_facet<APrintable>::mkobj(symtab.data());
                    std::string pretty = render_pretty(p, tc.margin_);

                    log && log(xtag("i_tc", i_tc), xtag("margin", tc.margin_), xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
                }
            }
        }

        TEST_CASE("DConstant-render", "[printable][DConstant]")
        {
            REQUIRE(s_init.evidence());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "DConstant-render"));

                for (std::size_t i_tc = 0, n_tc = s_constant_v.size(); i_tc < n_tc; ++i_tc) {
                    const auto & tc = s_constant_v[i_tc];

                    VarFixture fx(tc.label_);

                    DConstant * k = (tc.is_int_
                                     ? fx.make_constant(42L)
                                     : fx.make_constant(2.5));
                    REQUIRE(k != nullptr);

                    auto p = with_facet<APrintable>::mkobj(k);
                    std::string pretty = scrub_tseq(render_pretty(p, tc.margin_));

                    log && log(xtag("i_tc", i_tc), xtag("margin", tc.margin_), xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
                }
            }
        }

        /** what scrub_tseq hides, checked without depending on the numbers.
         *
         *  DConstant prints TWO typeseqs -- the boxed D-type's
         *  (value_._typeseq()) and the APrintable facet obj's
         *  (value_pr._typeseq()).  Two fields only earn their place if they can
         *  disagree.  **Observed 2026-08-10: they always agree**, because an
         *  obj<> carries the D-type's typeseq whichever facet it is viewed
         *  through, and FacetRegistry::variant() does not change the D-type.
         *
         *  So `:value.tseq` is REDUNDANT in the rendering.  Recorded here as a
         *  pinned property rather than left implicit: a mutation replacing
         *  value_pr._typeseq() with value_._typeseq() passes every other test
         *  in this file, and that is a fact about the printer, not a gap in the
         *  suite.  Dropping the field would be an output-visible change and
         *  wants its own commit -- see
         *  .xo-backlog/xo-printable2/issues/01-aprintable-pretty-ppsink.md
         **/
        TEST_CASE("DConstant-tseq-fields", "[printable][DConstant]")
        {
            REQUIRE(s_init.evidence());

            VarFixture fx_int("tseq.int");
            VarFixture fx_flt("tseq.flt");

            std::string s_int
                = render_pretty(with_facet<APrintable>::mkobj(fx_int.make_constant(42L)), 200);
            std::string s_flt
                = render_pretty(with_facet<APrintable>::mkobj(fx_flt.make_constant(2.5)), 200);

            CHECK(scrub_tseq(s_int) == "<DConstant :value_.tseq N :value.tseq N :value 42>");
            CHECK(first_tseq(s_int) > 0);

            /* the two fields agree -- the redundancy described above */
            CHECK(first_tseq(s_int) == second_tseq(s_int));
            CHECK(first_tseq(s_flt) == second_tseq(s_flt));

            /* ... and the boxed type IS genuinely discriminated between cases */
            CHECK(first_tseq(s_int) != first_tseq(s_flt));
        }

        TEST_CASE("DIfElseExpr-render", "[printable][DIfElseExpr]")
        {
            REQUIRE(s_init.evidence());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "DIfElseExpr-render"));

                for (std::size_t i_tc = 0, n_tc = s_ifelse_v.size(); i_tc < n_tc; ++i_tc) {
                    const auto & tc = s_ifelse_v[i_tc];

                    VarFixture fx(tc.label_);

                    DIfElseExpr * e = fx.make_ifelse(tc.with_test_, tc.with_true_,
                                                     tc.with_false_);
                    REQUIRE(e != nullptr);

                    auto p = with_facet<APrintable>::mkobj(e);
                    std::string pretty
                        = scrub_typevar(scrub_tseq(render_pretty(p, tc.margin_)));

                    log && log(xtag("i_tc", i_tc), xtag("margin", tc.margin_), xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
                }
            }
        }

        TEST_CASE("DSequenceExpr-render", "[printable][DSequenceExpr]")
        {
            REQUIRE(s_init.evidence());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "DSequenceExpr-render"));

                for (std::size_t i_tc = 0, n_tc = s_sequence_v.size(); i_tc < n_tc; ++i_tc) {
                    const auto & tc = s_sequence_v[i_tc];

                    VarFixture fx(tc.label_);

                    DSequenceExpr * e = fx.make_sequence(tc.n_elt_);
                    REQUIRE(e != nullptr);

                    auto p = with_facet<APrintable>::mkobj(e);
                    std::string pretty = scrub_tseq(render_pretty(p, tc.margin_));

                    log && log(xtag("i_tc", i_tc), xtag("margin", tc.margin_), xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
                }
            }
        }
        TEST_CASE("DDefineExpr-render", "[printable][DDefineExpr]")
        {
            REQUIRE(s_init.evidence());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "DDefineExpr-render"));

                for (std::size_t i_tc = 0, n_tc = s_define_v.size(); i_tc < n_tc; ++i_tc) {
                    const auto & tc = s_define_v[i_tc];

                    VarFixture fx(tc.label_);

                    DDefineExpr * e = fx.make_define(tc.name_, tc.with_rhs_);
                    REQUIRE(e != nullptr);

                    auto p = with_facet<APrintable>::mkobj(e);
                    std::string pretty
                        = scrub_typevar(scrub_tseq(render_pretty(p, tc.margin_)));

                    log && log(xtag("i_tc", i_tc), xtag("margin", tc.margin_), xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
                }
            }
        }
        TEST_CASE("DApplyExpr-render", "[printable][DApplyExpr]")
        {
            REQUIRE(s_init.evidence());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "DApplyExpr-render"));

                for (std::size_t i_tc = 0, n_tc = s_apply_v.size(); i_tc < n_tc; ++i_tc) {
                    const auto & tc = s_apply_v[i_tc];

                    VarFixture fx(tc.label_);

                    DApplyExpr * e = fx.make_applyexpr(tc.n_arg_);
                    REQUIRE(e != nullptr);

                    auto p = with_facet<APrintable>::mkobj(e);
                    std::string pretty
                        = scrub_type_id(scrub_typevar(scrub_tseq(render_pretty(p, tc.margin_))));

                    log && log(xtag("i_tc", i_tc), xtag("margin", tc.margin_), xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
                }
            }
        }
        TEST_CASE("DLocalSymtab-render", "[printable][DLocalSymtab]")
        {
            REQUIRE(s_init.evidence());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "DLocalSymtab-render"));

                for (std::size_t i_tc = 0, n_tc = s_localsymtab_v.size(); i_tc < n_tc; ++i_tc) {
                    const auto & tc = s_localsymtab_v[i_tc];

                    VarFixture fx(tc.label_);

                    DLocalSymtab * e = fx.make_localsymtab(tc.n_var_, 0 /*n_type*/);
                    REQUIRE(e != nullptr);

                    auto p = with_facet<APrintable>::mkobj(e);
                    std::string pretty
                        = scrub_type_id(scrub_typevar(scrub_tseq(render_pretty(p, tc.margin_))));

                    log && log(xtag("i_tc", i_tc), xtag("margin", tc.margin_), xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
                }
            }
        }

        /** the types_ path, which s_localsymtab_v cannot cover.
         *
         *  `.xo-backlog/xo-type/issues/01-no-aprintable-facet.md` recorded the
         *  throw as UNVERIFIED -- a code-read, since nothing constructed a
         *  symtab with a non-empty types_.  This is the observation, and it
         *  corrects where the ticket placed the fault: DLocalSymtab's own
         *  `(*types_)[i].to_facet<APrintable>()` SUCCEEDS, because DTypename
         *  has an IPrintable facet.  What throws is one level down, in
         *  DTypename::pretty's `type_.to_facet<APrintable>()`,
         *  since xo-type's D-types have none.
         *
         *  The two sides AGREED TO DISAGREE only briefly.  While DTypename was
         *  a phase-B stub, ppsink rendered here and legacy threw; once
         *  DTypename converted (same day, keeping to_facet<APrintable>) both
         *  throw, and the symmetry is back.  See DTypename-render for why the
         *  throw was kept rather than tolerated.
         *
         *  Both halves survive phase E, for the same reason as DTypename's.
         **/
        TEST_CASE("DLocalSymtab-types-throws", "[printable][DLocalSymtab]")
        {
            REQUIRE(s_init.evidence());

            VarFixture fx("types.throws");

            DLocalSymtab * e = fx.make_localsymtab(0 /*n_var*/, 1 /*n_type*/);
            REQUIRE(e != nullptr);

            auto p = with_facet<APrintable>::mkobj(e);

            /* the whole render dies rather than the one field, on both
             * protocols: DLocalSymtab's own facet lookup succeeds (types_
             * holds DTypenames, which HAVE the facet) and DTypename's does
             * not.  One level deeper than this ticket first claimed.
             */
            REQUIRE_THROWS_AS(render_pretty(p, 200), std::runtime_error);
        }
        TEST_CASE("DLambdaExpr-render", "[printable][DLambdaExpr]")
        {
            REQUIRE(s_init.evidence());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "DLambdaExpr-render"));

                for (std::size_t i_tc = 0, n_tc = s_lambda_v.size(); i_tc < n_tc; ++i_tc) {
                    const auto & tc = s_lambda_v[i_tc];

                    VarFixture fx(tc.label_);

                    DLambdaExpr * e = fx.make_lambda(tc.name_, tc.with_body_, tc.n_var_);
                    REQUIRE(e != nullptr);

                    auto p = with_facet<APrintable>::mkobj(e);
                    std::string pretty
                        = scrub_type_id(scrub_typevar(scrub_tseq(render_pretty(p, tc.margin_))));

                    log && log(xtag("i_tc", i_tc), xtag("margin", tc.margin_), xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
                }
            }
        }
        /** DTypename has NO renderable case, and that is the point.
         *
         *  RC's call, 2026-08-11: the printer keeps `to_facet<APrintable>()`
         *  and throws, rather than tolerating the missing facet with a
         *  placeholder.  The throw is a standing failing test for
         *  `.xo-backlog/xo-type/issues/01` -- a "yolo" red test, living in the
         *  tree rather than in a test file.  Retiring it with a placeholder
         *  would retire the only thing asserting the gap exists.
         *
         *  So this test pins the THROW, on both protocols.  It is expected to
         *  start failing the day xo-type gains an APrintable facet; that
         *  failure is the signal, and the fix then is to replace this with a
         *  rendering test.
         *
         *  Phase E deleted the matching deprecated assertion, as everywhere
         *  else in this file.  Here that assertion was NOT scaffolding for a
         *  pinned rendering -- there is no rendering -- it recorded that both
         *  protocols failed identically, which was the evidence that the
         *  conversion changed nothing.
         **/
        TEST_CASE("DTypename-render", "[printable][DTypename]")
        {
            REQUIRE(s_init.evidence());

            VarFixture fx("typename.throws");

            auto tn = fx.make_typename("t1", true /*with_type*/);
            auto pr = tn.to_facet<APrintable>();

            REQUIRE_THROWS_AS(render_pretty(pr, 200), std::runtime_error);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end printable_render.test.cpp */
