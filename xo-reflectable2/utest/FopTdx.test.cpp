/* file FopTdx.test.cpp
 *
 * author: Roland Conybeare, Sep 2026
 */

#include "xo/reflectable2/FopTdx.hpp"
#include "xo/reflectable2/Reflectable.hpp"
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
             * D-types: those arrive in issues/05, and this ticket must not
             * depend on that
             */
            struct DPoint {
                double x_;
                double y_;
            };

            /* AReflectable has no methods yet (issues/03 adds self_tp()),
             * so opting DPoint in is just the FacetImplementation mapping
             */
            class IReflectable_DPoint {};
        }
    } /*namespace ut*/

    namespace facet {
        template <>
        struct FacetImplementation<xo::reflect::AReflectable, xo::ut::DPoint> {
            using ImplType = xo::reflect::IReflectable_Xfer<xo::ut::DPoint,
                                                           xo::ut::IReflectable_DPoint>;
        };
    }

    namespace ut {
        using xo::reflect::AReflectable;
        using xo::facet::obj;

        namespace {
            /* reflect DPoint once; both test cases below need it */
            void require_dpoint_reflected() {
                static bool s_once = []() {
                    StructReflector<DPoint> sr;
                    REFLECT_MEMBER(sr, x);
                    REFLECT_MEMBER(sr, y);
                    sr.require_complete();
                    return true;
                }();

                (void)s_once;
            }
        }

        TEST_CASE("fop-reflects-as-a-pointer", "[reflectable2]") {
            require_dpoint_reflected();

            auto td = Reflect::require<obj<AReflectable, DPoint>>();

            REQUIRE(td->metatype() == Metatype::mt_pointer);
            /* 0 means "not known at compile time" -- a fop has a child
             * only when its data pointer is non-null
             */
            REQUIRE(td->n_child_fixed() == 0);
            REQUIRE(td->fixed_child_td(0) == Reflect::require<DPoint>());
        } /*TEST_CASE(fop-reflects-as-a-pointer)*/

        TEST_CASE("fop-child-is-its-representation", "[reflectable2]") {
            require_dpoint_reflected();

            DPoint pt{1.5, -2.5};
            obj<AReflectable, DPoint> o{&pt};

            TaggedPtr tp = Reflect::make_tp(&o);

            REQUIRE(tp.n_child() == 1);

            TaggedPtr child = tp.get_child(0);

            REQUIRE(child.td() == Reflect::require<DPoint>());
            REQUIRE(child.address() == &pt);
        } /*TEST_CASE(fop-child-is-its-representation)*/

        TEST_CASE("empty-fop-has-no-child", "[reflectable2]") {
            require_dpoint_reflected();

            obj<AReflectable, DPoint> o;

            TaggedPtr tp = Reflect::make_tp(&o);

            REQUIRE(tp.n_child() == 0);
        } /*TEST_CASE(empty-fop-has-no-child)*/

        TEST_CASE("erased-fop-reports-no-child-yet", "[reflectable2]") {
            /* the erased case is what issues/03 exists for.  Pinned here so
             * that ticket has a red test to turn green, rather than silently
             * changing behaviour nothing was watching.
             */
            require_dpoint_reflected();

            DPoint pt{1.5, -2.5};
            obj<AReflectable, DPoint> typed{&pt};
            obj<AReflectable> erased{typed};

            auto td = Reflect::require<obj<AReflectable>>();

            REQUIRE(td->metatype() == Metatype::mt_pointer);

            TaggedPtr tp = Reflect::make_tp(&erased);

            REQUIRE(tp.n_child() == 0);
        } /*TEST_CASE(erased-fop-reports-no-child-yet)*/
    } /*namespace ut*/
} /*namespace xo*/

/* end FopTdx.test.cpp */
