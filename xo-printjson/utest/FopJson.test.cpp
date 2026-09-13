/* file FopJson.test.cpp
 *
 * author: Roland Conybeare, Sep 2026
 */

#include "xo/printjson/PrintJson.hpp"
#include "xo/printjson/init_printjson.hpp"
#include <xo/reflectable2/FopTdx.hpp>
#include <xo/reflectable2/Reflectable.hpp>
#include <xo/facet/FacetRegistry.hpp>
/* NOT <xo/printable2/Printable.hpp>: that umbrella header pulls
 * <xo/alloc2/Allocator.hpp>, which printable2 does not declare a dependency
 * on.  See .xo-backlog/xo-printable2/issues/03.
 */
#include <xo/printable2/detail/APrintable.hpp>
#include <xo/printable2/detail/IPrintable_Any.hpp>
#include <xo/printable2/detail/IPrintable_Xfer.hpp>
#include <xo/printable2/detail/RPrintable.hpp>
#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/StructReflector.hpp>
#include <catch2/catch.hpp>
#include <sstream>

namespace xo {
    using xo::json::PrintJson;
    using xo::reflect::Reflect;
    using xo::reflect::StructReflector;
    using xo::reflect::TaggedPtr;

    namespace ut {
        InitEvidence s_fop_init_evidence = InitSubsys<S_printjson_tag>::require();

        namespace {
            /* throwaway representation; see xo-reflectable2/utest for the
             * reflection-side assertions on the same shape
             */
            struct DFopPoint {
                double x_;
                double y_;
            };

            class IReflectable_DFopPoint {
            public:
                static TaggedPtr self_tp(DFopPoint & self) {
                    return Reflect::make_tp(&self);
                }
            };

            /* reflectable, but deliberately NEVER register_impl'd -- see
             * print-obj-fast-path-skips-the-registry
             */
            struct DFopUnregistered {
                double v_;
            };

            class IReflectable_DFopUnregistered {
            public:
                static TaggedPtr self_tp(DFopUnregistered & self) {
                    return Reflect::make_tp(&self);
                }
            };

            /* printable but deliberately NOT reflectable */
            struct DFopOpaque {
                int n_;
            };

            class IPrintable_DFopOpaque {
            public:
                using PpSink = xo::print::APrintable::PpSink;

                static void pretty(const DFopOpaque & self, PpSink & sink) {
                    sink.put(self.n_ ? "DFopOpaque" : "DFopOpaque{}");
                }
            };

            /* reflectable, holding an ERASED fop member -- the routine shape,
             * see DDictionary/DList/DArray in xo-object2.  Its member facet is
             * APrintable so the member can be a representation that has not
             * opted in to reflection.
             */
            struct DFopBox {
                xo::facet::obj<xo::print::APrintable> inner_;
            };

            class IReflectable_DFopBox {
            public:
                static TaggedPtr self_tp(DFopBox & self) {
                    return Reflect::make_tp(&self);
                }
            };
        }
    } /*namespace ut*/

    namespace facet {
        template <>
        struct FacetImplementation<xo::reflect::AReflectable, xo::ut::DFopPoint> {
            using ImplType
                = xo::reflect::IReflectable_Xfer<xo::ut::DFopPoint,
                                                 xo::ut::IReflectable_DFopPoint>;
        };

        template <>
        struct FacetImplementation<xo::reflect::AReflectable, xo::ut::DFopBox> {
            using ImplType
                = xo::reflect::IReflectable_Xfer<xo::ut::DFopBox,
                                                 xo::ut::IReflectable_DFopBox>;
        };

        template <>
        struct FacetImplementation<xo::reflect::AReflectable,
                                   xo::ut::DFopUnregistered> {
            using ImplType
                = xo::reflect::IReflectable_Xfer<xo::ut::DFopUnregistered,
                                                 xo::ut::IReflectable_DFopUnregistered>;
        };

        template <>
        struct FacetImplementation<xo::print::APrintable, xo::ut::DFopOpaque> {
            using ImplType
                = xo::print::IPrintable_Xfer<xo::ut::DFopOpaque,
                                             xo::ut::IPrintable_DFopOpaque>;
        };
    }

    namespace ut {
        using xo::reflect::AReflectable;
        using xo::print::APrintable;
        using xo::facet::obj;

        namespace {
            void require_foppoint_reflected() {
                static bool s_once = []() {
                    /* the rotation is a runtime lookup: a FacetImplementation
                     * specialization alone does not register the pair
                     */
                    using xo::facet::FacetRegistry;

                    FacetRegistry::register_impl<AReflectable, DFopPoint>();
                    FacetRegistry::register_impl<AReflectable, DFopBox>();
                    FacetRegistry::register_impl<APrintable, DFopOpaque>();

                    StructReflector<DFopPoint> sr;
                    REFLECT_MEMBER(sr, x);
                    REFLECT_MEMBER(sr, y);
                    sr.require_complete();

                    StructReflector<DFopBox> br;
                    REFLECT_MEMBER(br, inner);
                    br.require_complete();

                    /* DELIBERATELY no register_impl for DFopUnregistered */
                    StructReflector<DFopUnregistered> ur;
                    REFLECT_MEMBER(ur, v);
                    ur.require_complete();

                    return true;
                }();

                (void)s_once;
            }
        }

        TEST_CASE("print-json-fop-object", "[printjson]") {
            require_foppoint_reflected();

            DFopPoint pt{1.5, -2.5};
            obj<AReflectable, DFopPoint> o{&pt};

            PrintJson print_json;

            /* a fop prints as its representation does, the same way
             * xo::ref::rp<T> prints as T does.  Compare against printing the
             * representation directly, rather than against a literal, so the
             * claim survives a change to struct formatting.
             */
            std::stringstream via_fop;
            print_json.print(Reflect::make_tp(&o), &via_fop);

            std::stringstream via_repr;
            print_json.print(Reflect::make_tp(&pt), &via_repr);

            REQUIRE(via_fop.str() == via_repr.str());
            REQUIRE(via_fop.str().find("\"x\"") != std::string::npos);
            REQUIRE(via_fop.str().find("1.5") != std::string::npos);
        } /*TEST_CASE(print-json-fop-object)*/

        TEST_CASE("print-json-erased-fop-object", "[printjson]") {
            /* the routine case: D-types hold erased fop members as a matter of
             * course, so this is the shape that matters, not the typed one
             */
            require_foppoint_reflected();

            DFopPoint pt{1.5, -2.5};
            obj<AReflectable, DFopPoint> typed{&pt};
            obj<AReflectable> erased{typed};

            PrintJson print_json;

            std::stringstream via_erased;
            print_json.print(Reflect::make_tp(&erased), &via_erased);

            std::stringstream via_repr;
            print_json.print(Reflect::make_tp(&pt), &via_repr);

            REQUIRE(via_erased.str() == via_repr.str());
        } /*TEST_CASE(print-json-erased-fop-object)*/

        TEST_CASE("print-json-empty-fop-object", "[printjson]") {
            require_foppoint_reflected();

            obj<AReflectable, DFopPoint> o;

            PrintJson print_json;

            std::stringstream ss;
            print_json.print(Reflect::make_tp(&o), &ss);

            /* print_generic_pointer emits {} when there are no children;
             * distinguishable from a struct because it has no _name_ member
             */
            REQUIRE(ss.str() == std::string("{}"));
        } /*TEST_CASE(print-json-empty-fop-object)*/

        TEST_CASE("print-obj-matches-a-hand-built-tagged-ptr", "[printjson]") {
            require_foppoint_reflected();

            DFopPoint pt{1.5, -2.5};
            obj<AReflectable, DFopPoint> o{&pt};

            PrintJson print_json;

            std::stringstream via_entry_point;
            print_json.print_obj(o, &via_entry_point);

            std::stringstream via_hand_built;
            print_json.print_tp(Reflect::make_tp(&o), &via_hand_built);

            REQUIRE(via_entry_point.str() == via_hand_built.str());
        } /*TEST_CASE(print-obj-matches-a-hand-built-tagged-ptr)*/

        TEST_CASE("print-obj-fast-path-agrees-with-the-generic-one", "[printjson]") {
            /* obj<AReflectable> takes the if-constexpr fast path -- straight to
             * self_tp(), no FacetRegistry probe and no pointer hop.  The two
             * routes reach the representation's TaggedPtr differently
             * (self_tp() vs establish_most_derived_tp), so only a test says
             * they agree.
             */
            require_foppoint_reflected();

            DFopPoint pt{1.5, -2.5};
            obj<AReflectable, DFopPoint> typed{&pt};
            obj<AReflectable> fast{typed};

            PrintJson print_json;

            std::stringstream via_fast;
            print_json.print_obj(fast, &via_fast);

            std::stringstream via_generic;
            print_json.print_tp(Reflect::make_tp(&pt), &via_generic);

            REQUIRE(via_fast.str() == via_generic.str());
        } /*TEST_CASE(print-obj-fast-path-agrees-with-the-generic-one)*/

        TEST_CASE("print-obj-renders-an-empty-fop-as-braces", "[printjson]") {
            /* the fast path's guard.  An EMPTY obj<AReflectable> carries
             * IReflectable_Any for its iface, whose self_tp() terminates, so
             * the fast path must fall through to the generic one.
             */
            require_foppoint_reflected();

            obj<AReflectable> empty;

            PrintJson print_json;

            std::stringstream ss;
            print_json.print_obj(empty, &ss);

            REQUIRE(ss.str() == std::string("{}"));
        } /*TEST_CASE(print-obj-renders-an-empty-fop-as-braces)*/

        TEST_CASE("print-obj-fast-path-skips-the-registry", "[printjson]") {
            /* The check that the fast path is actually TAKEN.  Comparing its
             * output against the generic path cannot show this -- they agree,
             * which is the point -- so discriminate on the one thing only the
             * fast path can do: an erased obj<AReflectable> already carries an
             * AReflectable implementation in its iface, so self_tp() needs no
             * FacetRegistry entry, while the generic path's rotation does.
             *
             * DFopUnregistered has the FacetImplementation mapping and no
             * register_impl<>() call.  If the fast path were removed, the
             * first REQUIRE below would throw.
             */
            require_foppoint_reflected();

            DFopUnregistered u{3.25};
            obj<AReflectable, DFopUnregistered> typed{&u};
            obj<AReflectable> erased{typed};

            PrintJson print_json;

            std::stringstream via_fast;
            REQUIRE_NOTHROW(print_json.print_obj(erased, &via_fast));
            REQUIRE(via_fast.str().find("3.25") != std::string::npos);

            /* same object, generic route: the rotation needs the registry */
            std::stringstream via_generic;
            REQUIRE_THROWS(print_json.print_tp(Reflect::make_tp(&erased),
                                               &via_generic));
        } /*TEST_CASE(print-obj-fast-path-skips-the-registry)*/

        TEST_CASE("validate-then-print-matches-print-alone", "[printjson]") {
            require_foppoint_reflected();

            DFopPoint pt{1.5, -2.5};
            obj<AReflectable, DFopPoint> o{&pt};

            PrintJson print_json;

            std::stringstream validated;
            print_json.validate_obj(o);
            print_json.print_obj(o, &validated);

            std::stringstream print_alone;
            print_json.print_obj(o, &print_alone);

            REQUIRE(validated.str() == print_alone.str());
        } /*TEST_CASE(validate-then-print-matches-print-alone)*/

        TEST_CASE("validation-throws-without-writing", "[printjson]") {
            /* why validate exists: print throws MID-traversal, leaving the
             * consumer holding a truncated document.  Validating first makes
             * it all-or-nothing.
             */
            require_foppoint_reflected();

            DFopOpaque op{7};
            obj<APrintable, DFopOpaque> opaque{&op};

            DFopBox box{obj<APrintable>{opaque}};

            PrintJson print_json;

            std::stringstream ss;

            REQUIRE_THROWS_WITH(print_json.validate_obj(obj<AReflectable,
                                                            DFopBox>{&box}),
                                Catch::Contains("DFopOpaque"));
            REQUIRE(ss.str().empty());

            /* and the thing being avoided: printing the same graph DOES write
             * before it throws
             */
            std::stringstream partial;
            REQUIRE_THROWS(print_json.print_obj(obj<AReflectable, DFopBox>{&box},
                                                &partial));
            REQUIRE_FALSE(partial.str().empty());
        } /*TEST_CASE(validation-throws-without-writing)*/
    } /*namespace ut*/
} /*namespace xo*/

/* end FopJson.test.cpp */
