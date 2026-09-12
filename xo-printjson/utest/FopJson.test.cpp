/* file FopJson.test.cpp
 *
 * author: Roland Conybeare, Sep 2026
 */

#include "xo/printjson/PrintJson.hpp"
#include "xo/printjson/init_printjson.hpp"
#include <xo/reflectable2/FopTdx.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/reflectable2/Reflectable.hpp>
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
        }
    } /*namespace ut*/

    namespace facet {
        template <>
        struct FacetImplementation<xo::reflect::AReflectable, xo::ut::DFopPoint> {
            using ImplType
                = xo::reflect::IReflectable_Xfer<xo::ut::DFopPoint,
                                                 xo::ut::IReflectable_DFopPoint>;
        };
    }

    namespace ut {
        using xo::reflect::AReflectable;
        using xo::facet::obj;

        namespace {
            void require_foppoint_reflected() {
                static bool s_once = []() {
                    /* the rotation is a runtime lookup: a FacetImplementation
                     * specialization alone does not register the pair
                     */
                    xo::facet::FacetRegistry::register_impl<AReflectable,
                                                            DFopPoint>();

                    StructReflector<DFopPoint> sr;
                    REFLECT_MEMBER(sr, x);
                    REFLECT_MEMBER(sr, y);
                    sr.require_complete();
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
    } /*namespace ut*/
} /*namespace xo*/

/* end FopJson.test.cpp */
