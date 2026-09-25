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

#include "printjson_utest_appcx.hpp"
#include "xo/printjson/PrintJson.hpp"
#include "xo/printjson/init_printjson.hpp"
#include <xo/reflectable2/FopTdx.hpp>
#include <xo/reflectable2/Reflectable.hpp>
#include <xo/facet/handlestore/ObjectSlot.hpp>
#include <xo/facet/handlestore/DHandleStore.hpp>
#include <xo/facet/AllocFlywheel.hpp>
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
        using xo::facet::AllocFlywheel;
        using xo::facet::DHandleStoreBase;
        using xo::facet::FacetRegistry;
        using xo::print::APrintable;
        using xo::mm::ArenaConfig;
        using xo::mm::ArenaNameStr;
        using xo::mm::DArena;

        namespace {
            void require_registered() {
                static bool s_once = []() {
                    FacetRegistry::register_impl<APrintable, DSlotProbe>();
                    return true;
                }();
                (void)s_once;
            }

            /** a flywheel, whose storage arena is header-enabled and aligned
             *  on the one agreed base alignment.
             *
             *  Was a bare DArena until 2026-09-24.  A non-empty ObjectSlot can
             *  now only be made by a DHandleStore, so a test that wants one
             *  has to have a store -- which is the point, since a storeless
             *  slot is exactly the unvouched-for state the change removes.
             **/
            rp<AllocFlywheel> make_flywheel(const char * tag) {
                ArenaConfig storage_cfg{ .name_ = ArenaNameStr::sprintf("%s.storage", tag),
                                         .size_ = 16*1024 };
                ArenaConfig strong_cfg { .name_ = ArenaNameStr::sprintf("%s.strong", tag),
                                         .size_ = 4*1024 };

                return AllocFlywheel::make_app(printjson_utest_facet_appcx(),
                                               storage_cfg, strong_cfg);
            }

            /** allocate a DSlotProbe from @p fw and adopt it as a root,
             *  exactly as DObjectHandle::make_strong_ref does.
             *
             *  @return a COPY of the slot the store made.  The store keeps the
             *  original; nothing here releases it, so the copy stays valid for
             *  the life of @p fw.
             **/
            ObjectSlot root(AllocFlywheel & fw, double x) {
                auto * mem = fw.storage().alloc(xo::reflect::typeseq::id<DSlotProbe>(),
                                                sizeof(DSlotProbe));
                REQUIRE(mem);

                auto * p = new (mem) DSlotProbe{x};
                auto typed = xo::facet::with_facet<APrintable>::mkobj(p);

                auto ref = fw.add_strong_ref
                    (static_cast<const xo::facet::ATop *>(typed.iface()),
                     typed.opaque_data());

                REQUIRE(ref.second);

                return *ref.second;
            }

            std::string render(PrintJson & pj, const ObjectSlot & slot) {
                std::stringstream ss;
                pj.print(slot, &ss);
                return ss.str();
            }
        }

        /* the restriction itself, checked by the compiler.  is_constructible
         * respects access, so this fails the moment the ctor goes public
         * again -- which is the whole of .xo-backlog/xo-facet/issues/04's
         * first claim.
         */
        static_assert(!std::is_constructible_v<ObjectSlot,
                                               const xo::facet::ATop *, void *>,
                      "a non-empty ObjectSlot must only be constructible by"
                      " DHandleStore -- see ObjectSlot's Provenance note");
        /* ..and the empty one stays available to anybody */
        static_assert(std::is_default_constructible_v<ObjectSlot>);

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

        TEST_CASE("occupied-slot-reports-identity-offset-and-size",
                  "[printjson][ObjectSlot]")
        {
            require_registered();

            PrintJson pj;
            auto fw = make_flywheel("utest.slot.one");

            ObjectSlot s0 = root(*fw.get(), 1.5);

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

            /* size comes from the ALLOC HEADER, not sizeof(DSlotProbe) -- the
             * two agree here only because DSlotProbe is fixed-size and needs
             * no padding.  Observed, not predicted.
             */
            REQUIRE(frame.find("\"size\": 8") != std::string::npos);
        } /*TEST_CASE(occupied-slot-reports-identity-offset-and-size)*/

        TEST_CASE("offset-is-resolved-from-the-pointer-alone", "[printjson][ObjectSlot]")
        {
            /* the point of the maskable base alignment: a slot knows where it
             * sits WITHOUT anyone handing it an arena.  Two arenas, and each
             * slot's offset is relative to its OWN -- which a printer that
             * captured a single base could not do.
             *
             * This is also what makes store-only slot creation worth the
             * trouble rather than just passing the arena down the print tree:
             * the slot is self-describing, so every consumer of
             * visit_object_slots gets the same answer without plumbing.
             */
            require_registered();

            PrintJson pj;
            auto fw1 = make_flywheel("utest.slot.a1");
            auto fw2 = make_flywheel("utest.slot.a2");

            /* fw2's first object is at the same offset as fw1's, from a
             * DIFFERENT base -- so equal offsets here mean each was resolved
             * against its own arena
             */
            ObjectSlot s1 = root(*fw1.get(), 1.5);
            ObjectSlot s2 = root(*fw2.get(), 2.5);

            REQUIRE(render(pj, s1).find("\"offset\": 16") != std::string::npos);
            REQUIRE(render(pj, s2).find("\"offset\": 16") != std::string::npos);

            /* second object in fw1 moves on; fw2 is untouched by it */
            ObjectSlot s3 = root(*fw1.get(), 3.5);

            REQUIRE(render(pj, s3).find("\"offset\": 16") == std::string::npos);
            REQUIRE(render(pj, s2).find("\"offset\": 16") != std::string::npos);
        } /*TEST_CASE(offset-is-resolved-from-the-pointer-alone)*/

        TEST_CASE("a-store-refuses-a-foreign-pointer", "[printjson][ObjectSlot]")
        {
            /* the runtime half of the provenance contract.  add_strong_ref's
             * precondition -- "x refers to memory owned by storage_" -- was a
             * doc comment until 2026-09-24; now the store checks it, which is
             * what lets the resulting slot vouch for itself.
             *
             * Allocate from ONE flywheel, offer it to ANOTHER.  The pointer is
             * perfectly valid; it just did not come from the store being asked
             * to adopt it.
             */
            require_registered();

            auto fw1 = make_flywheel("utest.slot.owner");
            auto fw2 = make_flywheel("utest.slot.stranger");

            auto * mem = fw1->storage().alloc(xo::reflect::typeseq::id<DSlotProbe>(),
                                              sizeof(DSlotProbe));
            REQUIRE(mem);

            auto * p = new (mem) DSlotProbe{1.5};
            auto typed = xo::facet::with_facet<APrintable>::mkobj(p);
            auto * iface = static_cast<const xo::facet::ATop *>(typed.iface());

            REQUIRE_THROWS(fw2->add_strong_ref(iface, typed.opaque_data()));

            /* and the rightful owner takes it */
            REQUIRE_NOTHROW(fw1->add_strong_ref(iface, typed.opaque_data()));
        } /*TEST_CASE(a-store-refuses-a-foreign-pointer)*/

        TEST_CASE("storage-base-align-is-write-once", "[printjson][ObjectSlot]")
        {
            /* the other half.  A slot's offset is masked with this value at
             * PRINT time, long after the store that validated an arena against
             * it was built -- so a silent reassignment would recover the wrong
             * arena and nothing would notice.
             *
             * Replaces `slot-without-an-agreed-alignment-reports-null-offset',
             * which set the value to 0 to exercise the printer's null-offset
             * branch.  That branch is now unreachable for a slot that exists
             * (a slot implies a store, a store implies a non-zero value, and
             * the value cannot change), so the test that reached it has become
             * the test that it CANNOT be reached.
             */
            const std::size_t in_force = DHandleStoreBase::storage_base_align();

            REQUIRE(in_force != 0);

            /* idempotent: several FacetAppcx agreeing is fine */
            REQUIRE_NOTHROW(DHandleStoreBase::assign_storage_base_align(in_force));

            REQUIRE_THROWS(DHandleStoreBase::assign_storage_base_align(0));
            REQUIRE_THROWS(DHandleStoreBase::assign_storage_base_align(in_force * 2));

            /* refused, not quietly recorded */
            REQUIRE(DHandleStoreBase::storage_base_align() == in_force);
        } /*TEST_CASE(storage-base-align-is-write-once)*/
    } /*namespace ut*/
} /*namespace xo*/

/* end ObjectSlotJson.test.cpp */
