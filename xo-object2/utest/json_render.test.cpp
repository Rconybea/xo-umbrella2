/* @file json_render.test.cpp
 *
 * author: Roland Conybeare, Sep 2026
 *
 * JSON rendering for object2's D-types, via xo-printjson.
 *
 * The pairing to keep in view: DFloat::reflect_self() describes the BOX
 * faithfully -- a struct with a `value` member -- while
 * SetupObject2::provide_json_printers installs a printer that renders it as a
 * bare number.  Both are asserted here, because the whole point of separating
 * them is that they say different things.
 *
 * Expectations are OBSERVED, never predicted.
 */

#include <xo/object2/DFloat.hpp>
#include <xo/object2/SetupObject2.hpp>
#include <xo/object2/number/IReflectable_DFloat.hpp>
#include <xo/object2/number/IGCObject_DFloat.hpp>
#include <xo/alloc2/arena/IAllocator_DArena.hpp>
#include <xo/printjson/PrintJson.hpp>
#include <xo/reflectable2/FopTdx.hpp>
#include <xo/reflect/Reflect.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <string>

namespace xo {
    using xo::scm::DFloat;
    using xo::scm::SetupObject2;
    using xo::json::PrintJson;
    using xo::reflect::AReflectable;
    using xo::reflect::Metatype;
    using xo::reflect::Reflect;
    using xo::mm::AAllocator;
    using xo::mm::AGCObject;
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;
    using xo::mm::ArenaNameStr;
    using xo::facet::with_facet;

    namespace ut {
        namespace {
            /** object2's registrations, once. **/
            void require_object2_setup(PrintJson * pjson) {
                REQUIRE(SetupObject2::register_facets());

                SetupObject2::reflect_types();
                SetupObject2::provide_json_printers(pjson);
            }
        }

        TEST_CASE("DFloat-reflects-its-layout", "[printjson][DFloat]")
        {
            PrintJson print_json;
            require_object2_setup(&print_json);

            auto td = Reflect::require<DFloat>();

            /* reflection describes the BOX -- a struct with one member.  If
             * this ever reads mt_atomic, reflect_self() did not run, and the
             * json below would still pass, since the printer short-circuits
             * the metatype switch.
             */
            REQUIRE(td->metatype() == Metatype::mt_struct);
            REQUIRE(td->n_child_fixed() == 1);
            REQUIRE(td->struct_member_name(0) == std::string("value"));
        } /*TEST_CASE(DFloat-reflects-its-layout)*/

        TEST_CASE("DFloat-renders-as-a-bare-number", "[printjson][DFloat]")
        {
            PrintJson print_json;
            require_object2_setup(&print_json);

            ArenaConfig cfg { .name_ = ArenaNameStr::sprintf("utest.json.float"),
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            auto x = with_facet<AReflectable>::mkobj(DFloat::_box(alloc, 1.5));

            std::stringstream ss;
            print_json.print_obj(x, &ss);

            /* NOT {"_name_": "DFloat", "value": 1.5}: the printer decides how a
             * boxed float reads, and reflection is left honest about layout
             */
            REQUIRE(ss.str() == std::string("1.5"));
        } /*TEST_CASE(DFloat-renders-as-a-bare-number)*/

        TEST_CASE("erased-DFloat-renders-the-same", "[printjson][DFloat]")
        {
            /* the routine shape: a D-type's member is obj<AGCObject>, not a
             * typed fop.  Reaching the printer from there needs the rotation
             * through AReflectable that xo-reflectable2 provides.
             */
            PrintJson print_json;
            require_object2_setup(&print_json);

            ArenaConfig cfg { .name_ = ArenaNameStr::sprintf("utest.json.erased"),
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            auto gco = with_facet<AGCObject>::mkobj(DFloat::_box(alloc, 1.5));

            std::stringstream ss;
            print_json.print_obj(gco, &ss);

            REQUIRE(ss.str() == std::string("1.5"));
        } /*TEST_CASE(erased-DFloat-renders-the-same)*/
    } /*namespace ut*/
} /*namespace xo*/

/* end json_render.test.cpp */
