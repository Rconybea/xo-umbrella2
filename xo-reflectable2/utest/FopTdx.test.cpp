/* file FopTdx.test.cpp
 *
 * author: Roland Conybeare, Sep 2026
 */

#include "xo/reflectable2/FopTdx.hpp"
#include "xo/reflectable2/Reflectable.hpp"
/* NOT <xo/printable2/Printable.hpp>: that umbrella header pulls
 * <xo/alloc2/Allocator.hpp>, which printable2 does not declare a dependency on.
 * See .xo-backlog/xo-printable2/issues/.
 */
#include <xo/printable2/detail/APrintable.hpp>
#include <xo/printable2/detail/IPrintable_Any.hpp>
#include <xo/printable2/detail/IPrintable_Xfer.hpp>
#include <xo/printable2/detail/RPrintable.hpp>
#include <xo/reflect/StructReflector.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::reflect::FopTdx;
    using xo::reflect::Metatype;
    using xo::reflect::Reflect;
    using xo::reflect::StructReflector;
    using xo::reflect::TaggedPtr;

    namespace ut {
        namespace {
            /* throwaway representation, deliberately not one of xo-object2's
             * D-types: those are issues/05, and this ticket must not depend on
             * it
             */
            struct DPoint {
                double x_;
                double y_;
            };

            class IReflectable_DPoint {
            public:
                /* what every opting-in type's implementation amounts to */
                static TaggedPtr self_tp(DPoint & self) {
                    return Reflect::make_tp(&self);
                }
            };

            /* printable but deliberately NOT reflectable -- the state every
             * xo-object2 D-type is in before issues/05 converts it
             */
            struct DOpaque {
                int n_;
            };

            class IPrintable_DOpaque {
            public:
                using PpSink = xo::print::APrintable::PpSink;

                static void pretty(const DOpaque & self, PpSink & sink) {
                    sink.put(self.n_ ? "DOpaque" : "DOpaque{}");
                }
            };
        }
    } /*namespace ut*/

    namespace facet {
        template <>
        struct FacetImplementation<xo::reflect::AReflectable, xo::ut::DPoint> {
            using ImplType = xo::reflect::IReflectable_Xfer<xo::ut::DPoint,
                                                           xo::ut::IReflectable_DPoint>;
        };

        template <>
        struct FacetImplementation<xo::print::APrintable, xo::ut::DOpaque> {
            using ImplType = xo::print::IPrintable_Xfer<xo::ut::DOpaque,
                                                        xo::ut::IPrintable_DOpaque>;
        };
    }

    namespace ut {
        using xo::reflect::AReflectable;
        using xo::print::APrintable;
        using xo::facet::obj;

        namespace {
            /* a D-type holding an ERASED fop member, which is the routine
             * shape -- see DDictionary, DList, DArray in xo-object2
             */
            struct DHolder {
                obj<AReflectable> inner_;
            };

            void require_reflected() {
                static bool s_once = []() {
                    /* a FacetImplementation specialization is NOT enough: the
                     * rotation is a RUNTIME lookup, so the pair has to be
                     * registered.  register_impl() also registers both types
                     * with TypeRegistry, which is what lets a failure name the
                     * representation instead of reporting a bare typeseq.
                     */
                    using xo::facet::FacetRegistry;

                    FacetRegistry::register_impl<AReflectable, DPoint>();
                    FacetRegistry::register_impl<APrintable, DOpaque>();

                    StructReflector<DPoint> sr;
                    REFLECT_MEMBER(sr, x);
                    REFLECT_MEMBER(sr, y);
                    sr.require_complete();

                    StructReflector<DHolder> hr;
                    REFLECT_MEMBER(hr, inner);
                    hr.require_complete();

                    return true;
                }();

                (void)s_once;
            }
        }

        TEST_CASE("fop-reflects-as-a-pointer", "[reflectable2]") {
            require_reflected();

            auto td = Reflect::require<obj<AReflectable, DPoint>>();

            REQUIRE(td->metatype() == Metatype::mt_pointer);
            /* 0 means "not known at compile time" -- a fop has a child only
             * when its data pointer is non-null
             */
            REQUIRE(td->n_child_fixed() == 0);
            REQUIRE(td->fixed_child_td(0) == Reflect::require<DPoint>());
        } /*TEST_CASE(fop-reflects-as-a-pointer)*/

        TEST_CASE("fop-child-is-its-representation", "[reflectable2]") {
            require_reflected();

            DPoint pt{1.5, -2.5};
            obj<AReflectable, DPoint> o{&pt};

            TaggedPtr tp = Reflect::make_tp(&o);

            REQUIRE(tp.n_child() == 1);

            TaggedPtr child = tp.get_child(0);

            REQUIRE(child.td() == Reflect::require<DPoint>());
            REQUIRE(child.address() == &pt);
        } /*TEST_CASE(fop-child-is-its-representation)*/

        TEST_CASE("empty-fop-has-no-child", "[reflectable2]") {
            require_reflected();

            obj<AReflectable, DPoint> o;

            TaggedPtr tp = Reflect::make_tp(&o);

            REQUIRE(tp.n_child() == 0);
        } /*TEST_CASE(empty-fop-has-no-child)*/

        TEST_CASE("erased-fop-resolves-to-its-representation", "[reflectable2]") {
            require_reflected();

            DPoint pt{1.5, -2.5};
            obj<AReflectable, DPoint> typed{&pt};
            obj<AReflectable> erased{typed};

            auto td = Reflect::require<obj<AReflectable>>();

            REQUIRE(td->metatype() == Metatype::mt_pointer);

            TaggedPtr tp = Reflect::make_tp(&erased);

            /* the rotation through FacetRegistry to AReflectable happens here */
            REQUIRE(tp.n_child() == 1);

            TaggedPtr child = tp.get_child(0);

            REQUIRE(child.td() == Reflect::require<DPoint>());
            REQUIRE(child.address() == &pt);
        } /*TEST_CASE(erased-fop-resolves-to-its-representation)*/

        TEST_CASE("erased-fop-resolves-via-most-derived-self-tp", "[reflectable2]") {
            /* the OTHER entry point: StructMember::get_tp() calls this hook,
             * which is how a nested erased member is reached
             */
            require_reflected();

            DPoint pt{1.5, -2.5};
            obj<AReflectable, DPoint> typed{&pt};
            obj<AReflectable> erased{typed};

            auto td = Reflect::require<obj<AReflectable>>();

            TaggedPtr tp = td->most_derived_self_tp(&erased);

            REQUIRE(tp.td() == Reflect::require<DPoint>());
            REQUIRE(tp.address() == &pt);
        } /*TEST_CASE(erased-fop-resolves-via-most-derived-self-tp)*/

        TEST_CASE("empty-erased-fop-does-not-rotate", "[reflectable2]") {
            /* no representation to resolve to; must not throw looking for one */
            require_reflected();

            obj<AReflectable> erased;

            auto td = Reflect::require<obj<AReflectable>>();

            REQUIRE(td->most_derived_self_tp(&erased).td() == td);
            REQUIRE(Reflect::make_tp(&erased).n_child() == 0);
        } /*TEST_CASE(empty-erased-fop-does-not-rotate)*/

        TEST_CASE("struct-member-reaches-an-erased-fop", "[reflectable2]") {
            require_reflected();

            DPoint pt{1.5, -2.5};
            obj<AReflectable, DPoint> typed{&pt};
            DHolder holder{obj<AReflectable>{typed}};

            TaggedPtr tp = Reflect::make_tp(&holder);

            REQUIRE(tp.n_child() == 1);
            REQUIRE(tp.td()->struct_member_name(0) == std::string("inner"));

            /* StructMember::get_tp() routes through most_derived_self_tp, so
             * the member arrives already resolved to its representation
             */
            TaggedPtr member = tp.get_child(0);

            REQUIRE(member.td() == Reflect::require<DPoint>());
            REQUIRE(member.address() == &pt);
        } /*TEST_CASE(struct-member-reaches-an-erased-fop)*/

        TEST_CASE("a-representation-that-has-not-opted-in-throws", "[reflectable2]") {
            /* DOpaque implements APrintable and nothing else.  Rendering it as
             * empty would be a silent wrong answer, so the rotation throws --
             * and the message must name the representation, not hand the
             * reader a bare typeseq number to go look up.
             */
            require_reflected();

            DOpaque op{7};
            obj<APrintable, DOpaque> typed{&op};
            obj<APrintable> erased{typed};

            TaggedPtr tp = Reflect::make_tp(&erased);

            /* counting needs no rotation, so the throw lands on the FETCH --
             * which is where printjson's print_generic_pointer goes next
             */
            REQUIRE(tp.n_child() == 1);
            REQUIRE_THROWS_WITH(tp.get_child(0), Catch::Contains("DOpaque"));
        } /*TEST_CASE(a-representation-that-has-not-opted-in-throws)*/
    } /*namespace ut*/
} /*namespace xo*/

/* end FopTdx.test.cpp */
