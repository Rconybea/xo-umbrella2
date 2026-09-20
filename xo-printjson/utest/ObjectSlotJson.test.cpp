/* file ObjectSlotJson.test.cpp
 *
 * author: Roland Conybeare, Sep 2026
 *
 * json rendering for xo::facet::ObjectSlot -- a flywheel root slot.
 *
 * The shape to keep in view: an ObjectSlot is an erased fop, so reflection
 * CANNOT describe it.  FopTdx's erased path rotates through AReflectable to
 * render the representation's contents, and throws for a representation that
 * has not opted in.  ObjectSlot escapes that only because it DERIVES from
 * obj<ATop> rather than aliasing it, so it does not match
 * EstablishTdx<obj<AFacet,DRepr>> and reflects as an opaque atom instead --
 * which a bespoke printer then takes over.  That is what the shim is for, and
 * `slot-does-not-take-the-erased-fop-path' below is what pins it.
 *
 * Expectations are OBSERVED, never predicted.
 */

#include "xo/printjson/PrintJson.hpp"
#include "xo/printjson/init_printjson.hpp"
/* the obj<ATop> half of slot-does-not-take-the-erased-fop-path instantiates
 * FopTdx's erased branch, which rotates to AReflectable -- so its router has
 * to be visible.  Same reason FopJson.test.cpp includes these.
 */
#include <xo/reflectable2/FopTdx.hpp>
#include <xo/reflectable2/Reflectable.hpp>
#include <xo/facet/handlestore/ObjectSlot.hpp>
#include <xo/facet/handlestore/DHandleStore.hpp>
#include <xo/facet/TypeRegistry.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/printable2/detail/APrintable.hpp>
#include <xo/printable2/detail/IPrintable_Any.hpp>
#include <xo/printable2/detail/IPrintable_Xfer.hpp>
#include <xo/printable2/detail/RPrintable.hpp>
#include <xo/arena/DArena.hpp>
#include <xo/reflect/Reflect.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <string>

namespace xo {
    using xo::json::PrintJson;
    using xo::reflect::Metatype;
    using xo::reflect::Reflect;

    namespace ut {
        namespace {
            /** throwaway representation, printable so an ObjectSlot can be
             *  built the way DObjectHandle::make_strong_ref builds one.
             *
             *  NB being in an anonymous namespace puts "{anonymous}" in the
             *  name TypeRegistry reports, which the assertions below spell out.
             *  That is also the spelling two TUs would SHARE -- see
             *  .xo-backlog/xo-facet/issues/01, where internal-linkage types are
             *  deliberately excluded from name-keyed id allocation for exactly
             *  that reason.
             **/
            struct DSlotProbe {
                double value_;
            };

            class IPrintable_DSlotProbe {
            public:
                using PpSink = xo::print::APrintable::PpSink;

                static void pretty(const DSlotProbe & self, PpSink & sink) {
                    sink.pp(self.value_);
                }
            };
        }
    } /*namespace ut*/

    namespace facet {
        template <>
        struct FacetImplementation<xo::print::APrintable, xo::ut::DSlotProbe> {
            using ImplType = xo::print::IPrintable_Xfer<xo::ut::DSlotProbe,
                                                        xo::ut::IPrintable_DSlotProbe>;
        };
    }

    namespace ut {
        using xo::facet::ObjectSlot;
        using xo::facet::DHandleStoreBase;
        using xo::facet::FacetRegistry;
        using xo::print::APrintable;
        using xo::mm::ArenaConfig;
        using xo::mm::ArenaNameStr;
        using xo::mm::DArena;

        namespace {
            /** every participating arena must share this; see
             *  DHandleStoreBase::storage_base_align
             **/
            constexpr std::size_t c_align = 2UL * 1024 * 1024 * 1024;

            void require_registered() {
                static bool s_once = []() {
                    FacetRegistry::register_impl<APrintable, DSlotProbe>();
                    DHandleStoreBase::assign_storage_base_align(c_align);
                    return true;
                }();
                (void)s_once;
            }

            /** an arena configured the way a flywheel's storage is **/
            DArena make_storage(const char * name) {
                return DArena::map(ArenaConfig()
                                   .with_name(ArenaNameStr::from_cstr(name))
                                   .with_size(1UL * 1024 * 1024)
                                   .with_base_align_z(c_align)
                                   .with_exclusive_block_flag(true)
                                   .with_store_header_flag(true));
            }

            /** allocate a DSlotProbe from @p arena and erase it into a slot,
             *  exactly as DObjectHandle::make_strong_ref does
             **/
            ObjectSlot root(DArena & arena, double x) {
                auto * mem = arena.alloc(xo::reflect::typeseq::id<DSlotProbe>(),
                                         sizeof(DSlotProbe));
                REQUIRE(mem);

                auto * p = new (mem) DSlotProbe{x};
                auto typed = xo::facet::with_facet<APrintable>::mkobj(p);

                return ObjectSlot(static_cast<const xo::facet::ATop *>(typed.iface()),
                                  typed.opaque_data());
            }

            std::string render(PrintJson & pj, const ObjectSlot & slot) {
                std::stringstream ss;
                pj.print(slot, &ss);
                return ss.str();
            }
        }

        TEST_CASE("slot-does-not-take-the-erased-fop-path", "[printjson][ObjectSlot]")
        {
            /* the property the shim exists for.  obj<ATop> reflects through
             * FopTdx as a POINTER, whose erased branch rotates to AReflectable
             * and throws for an unregistered representation.  ObjectSlot is a
             * distinct type, so it gets the unreflected default instead -- an
             * atom -- and the bespoke printer keys on that.
             *
             * If this ever reads mt_pointer, ObjectSlot has become an alias
             * again and every render below is one throw away.
             */
            REQUIRE(Reflect::require<ObjectSlot>()->metatype() == Metatype::mt_atomic);
            REQUIRE(Reflect::require<xo::facet::obj<xo::facet::ATop>>()->metatype()
                    == Metatype::mt_pointer);
        } /*TEST_CASE(slot-does-not-take-the-erased-fop-path)*/

        TEST_CASE("empty-slot-renders-as-null", "[printjson][ObjectSlot]")
        {
            require_registered();

            PrintJson pj;
            ObjectSlot empty;

            /* null rather than omitted: slots are emitted including the empty
             * ones, so a consumer reads a slot's index from its position in
             * the enclosing array
             */
            REQUIRE(render(pj, empty) == std::string("null"));
        } /*TEST_CASE(empty-slot-renders-as-null)*/

        TEST_CASE("occupied-slot-reports-identity-and-offset", "[printjson][ObjectSlot]")
        {
            require_registered();

            PrintJson pj;
            DArena arena = make_storage("utest.slot.one");

            ObjectSlot s0 = root(arena, 1.5);

            const std::string frame = render(pj, s0);

            INFO("frame: " << frame);

            REQUIRE(frame.find("\"typeseq\": ") != std::string::npos);
            REQUIRE(frame.find("\"type\": \"xo::ut::{anonymous}::DSlotProbe\"")
                    != std::string::npos);

            /* 16 == the arena preamble (back pointer) + one AllocHeader.  The
             * number is a consequence of storage being header-enabled, which
             * DHandleStore requires; if it reads 0 the preamble vanished.
             */
            REQUIRE(frame.find("\"offset\": 16") != std::string::npos);
        } /*TEST_CASE(occupied-slot-reports-identity-and-offset)*/

        TEST_CASE("offset-is-resolved-from-the-pointer-alone", "[printjson][ObjectSlot]")
        {
            /* the point of the maskable base alignment: a slot knows where it
             * sits WITHOUT anyone handing it an arena.  Two arenas, and each
             * slot's offset is relative to its OWN -- which a printer that
             * captured a single base could not do.
             */
            require_registered();

            PrintJson pj;
            DArena a1 = make_storage("utest.slot.a1");
            DArena a2 = make_storage("utest.slot.a2");

            /* a2's first object is at the same offset as a1's, from a
             * DIFFERENT base -- so equal offsets here mean each was resolved
             * against its own arena
             */
            ObjectSlot s1 = root(a1, 1.5);
            ObjectSlot s2 = root(a2, 2.5);

            REQUIRE(render(pj, s1).find("\"offset\": 16") != std::string::npos);
            REQUIRE(render(pj, s2).find("\"offset\": 16") != std::string::npos);

            /* second object in a1 moves on; a2 is untouched by it */
            ObjectSlot s3 = root(a1, 3.5);

            REQUIRE(render(pj, s3).find("\"offset\": 16") == std::string::npos);
            REQUIRE(render(pj, s2).find("\"offset\": 16") != std::string::npos);
        } /*TEST_CASE(offset-is-resolved-from-the-pointer-alone)*/

        TEST_CASE("slot-without-an-agreed-alignment-reports-null-offset",
                  "[printjson][ObjectSlot]")
        {
            /* storage_base_align() is 0 until a FacetAppcx has been built.
             * Masking with ~(0-1) == 0 would dereference the result, so the
             * printer reports the absence instead.  Restored afterwards --
             * the value is process-wide state.
             */
            require_registered();

            const std::size_t saved = DHandleStoreBase::storage_base_align();

            PrintJson pj;
            DArena arena = make_storage("utest.slot.noalign");
            ObjectSlot s0 = root(arena, 1.5);

            DHandleStoreBase::assign_storage_base_align(0);

            const std::string frame = render(pj, s0);

            DHandleStoreBase::assign_storage_base_align(saved);

            INFO("frame: " << frame);

            REQUIRE(frame.find("\"offset\": null") != std::string::npos);
            /* identity still reported -- it needs no alignment */
            REQUIRE(frame.find("\"type\": \"xo::ut::{anonymous}::DSlotProbe\"")
                    != std::string::npos);
        } /*TEST_CASE(slot-without-an-agreed-alignment-reports-null-offset)*/
    } /*namespace ut*/
} /*namespace xo*/

/* end ObjectSlotJson.test.cpp */
