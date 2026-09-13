/* @file json_render.test.cpp
 *
 * author: Roland Conybeare, Sep 2026
 *
 * JSON rendering for stringtable2's D-types, via xo-printjson.
 *
 * Companion to xo-object2/utest/json_render.test.cpp, and the pairing is the
 * OPPOSITE one there.  DFloat reflects faithfully as a struct and a printer
 * overrides that with a bare number; DString reflects as an opaque atom -- it
 * ends in a flexible array, so StructReflector cannot describe its contents --
 * and the printer is the only thing that carries the characters.  Both facts
 * are asserted below, because the second follows from the first.
 *
 * Expectations are OBSERVED, never predicted.
 */

#include <xo/stringtable2/DString.hpp>
#include <xo/stringtable2/SetupStringtable2.hpp>
#include <xo/stringtable2/string/IReflectable_DString.hpp>
#include <xo/stringtable2/string/IGCObject_DString.hpp>
#include <xo/alloc2/arena/IAllocator_DArena.hpp>
#include <xo/printjson/PrintJson.hpp>
#include <xo/reflectable2/FopTdx.hpp>
#include <xo/reflect/Reflect.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <string>

namespace xo {
    using xo::scm::DString;
    using xo::scm::SetupStringtable2;
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
    using xo::facet::vt;

    namespace ut {
        namespace {
            /** stringtable2's registrations, once.
             *
             *  No reflect_types() counterpart to SetupObject2's: there is
             *  nothing member-wise to say about DString.
             **/
            void require_stringtable2_setup(PrintJson * pjson) {
                REQUIRE(SetupStringtable2::register_facets());

                SetupStringtable2::provide_json_printers(pjson);
            }
        }

        TEST_CASE("DString-reflects-as-an-atom", "[printjson][DString]")
        {
            PrintJson print_json;
            require_stringtable2_setup(&print_json);

            auto td = Reflect::require<DString>();

            /* the unreflected default (AtomicTdx), and deliberately so: a
             * flexible array member has an incomplete type, so the only
             * member-wise description available would be {capacity_, size_},
             * which describes the header and omits the payload.
             */
            REQUIRE(td->metatype() == Metatype::mt_atomic);
            REQUIRE(td->n_child_fixed() == 0);
        } /*TEST_CASE(DString-reflects-as-an-atom)*/

        TEST_CASE("DString-renders-as-a-json-string", "[printjson][DString]")
        {
            PrintJson print_json;
            require_stringtable2_setup(&print_json);

            ArenaConfig cfg { .name_ = ArenaNameStr::sprintf("utest.json.string"),
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            auto x = with_facet<AReflectable>::mkobj(DString::from_cstr(alloc, "hello"));

            std::stringstream ss;
            print_json.print_obj(x, &ss);

            REQUIRE(ss.str() == std::string("\"hello\""));
        } /*TEST_CASE(DString-renders-as-a-json-string)*/

        TEST_CASE("erased-DString-renders-the-same", "[printjson][DString]")
        {
            /* the routine shape: a D-type's member is obj<AGCObject>, not a
             * typed fop.  Reaching the printer from there needs the rotation
             * through AReflectable that xo-reflectable2 provides -- and hence
             * the register_impl<AReflectable, DString>() in register_facets,
             * without which this throws rather than fails.
             */
            PrintJson print_json;
            require_stringtable2_setup(&print_json);

            ArenaConfig cfg { .name_ = ArenaNameStr::sprintf("utest.json.erased"),
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            /* vt<>, NOT with_facet<>::mkobj -- mkobj hands back a TYPED
             * obj<AGCObject,DString> (obj.hpp:165), which FopTdx resolves at
             * compile time and which therefore never reaches the rotation.
             * The conversion below is what erases it.
             */
            vt<AGCObject> gco
                = with_facet<AGCObject>::mkobj(DString::from_cstr(alloc, "hello"));

            std::stringstream ss;
            print_json.print_obj(gco, &ss);

            REQUIRE(ss.str() == std::string("\"hello\""));
        } /*TEST_CASE(erased-DString-renders-the-same)*/

        TEST_CASE("DString-json-uses-size-not-nul", "[printjson][DString]")
        {
            /* DString::operator std::string_view() stops at the first null;
             * size_ is the authority on extent.  The printer uses size_, so
             * the trailing 'b' survives rather than being truncated away --
             * the EXTENT is stringtable2's half of this, and is what this case
             * exists to pin.
             *
             * The escaping is xo-ppsink's: JsonPrinter_string renders through
             * quot(), so PrintJson.cpp:367's "TODO: escapes special
             * characters" is stale.  Two notes on what comes back, neither of
             * them stringtable2's to fix:
             *   - \x00 is ppsink's escape vocabulary, not JSON's, which wants
             *     \u0000.  A strict parser rejects it.
             *   - so this expectation will change when printjson grows a
             *     json-specific escape.  It is recorded as OBSERVED, and a
             *     failure here means that happened -- not that DString broke.
             */
            PrintJson print_json;
            require_stringtable2_setup(&print_json);

            ArenaConfig cfg { .name_ = ArenaNameStr::sprintf("utest.json.embnul"),
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            auto x = with_facet<AReflectable>::mkobj
                (DString::from_view(alloc, std::string_view("a\0b", 3)));

            std::stringstream ss;
            print_json.print_obj(x, &ss);

            REQUIRE(ss.str() == std::string("\"a\\x00b\""));
        } /*TEST_CASE(DString-json-uses-size-not-nul)*/
    } /*namespace ut*/
} /*namespace xo*/

/* end json_render.test.cpp */
