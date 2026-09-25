/* @file flywheel_frame.test.cpp
 *
 * author: Roland Conybeare, Sep 2026
 *
 * JSON rendering of an AllocFlywheel's state -- one animation frame.
 *
 * NOT object2's subject.  AllocFlywheel is xo-facet's, and the printers are
 * xo-printjson's.  This lives here because it is the lowest place a flywheel
 * can be filled with a REAL representation (DFloat) and then rendered --
 * xo-printjson's own utest has facet but no D-type worth putting in a slot.
 *
 * What is pinned here is a WIRE CONTRACT: the key names below are what a
 * consumer outside this process parses.  So they are asserted literally rather
 * than derived from the c++ member names -- a member rename must not silently
 * rename a key.
 *
 * Expectations are OBSERVED, never predicted.
 */

#include <xo/object2/Float.hpp>   /* DFloat + IPrintable_DFloat */
#include <xo/object2/SetupObject2.hpp>
#include <xo/object2/number/IGCObject_DFloat.hpp>
#include <xo/facet/ObjectHandle.hpp>
#include <xo/facet/AllocFlywheel.hpp>
#include <xo/printable2/Printable.hpp>
#include <xo/alloc2/arena/IAllocator_DArena.hpp>
#include <xo/facet/cx/FacetAppcx.hpp>
#include <xo/indentlog2/cx/Indentlog2Appcx.hpp>
#include <xo/printjson/PrintJson.hpp>
#include <xo/reflect/Reflect.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <regex>
#include <string>

namespace xo {
    using xo::scm::DFloat;
    using xo::scm::SetupObject2;
    using xo::json::PrintJson;
    using xo::facet::AllocFlywheel;
    using xo::facet::DObjectHandle;
    using xo::facet::with_facet;
    using xo::mm::ArenaConfig;
    using xo::mm::ArenaNameStr;
    using xo::reflect::Metatype;
    using xo::reflect::Reflect;
    using xo::mm::AAllocator;
    using xo::print::APrintable;

    namespace ut {
        namespace {
            using FrameAppConfig = AppConfig<S_indentlog2_tag, S_facet_tag>;
            using FrameAppContext = AppContext<S_indentlog2_tag, S_facet_tag>;

            /** a facet context for this file's flywheels.
             *
             *  Local rather than shared: xo-facet's FacetUtestAppcx is not
             *  installed (it is test-local to that subsystem), so object2's
             *  tests cannot borrow it.
             **/
            FacetAppcx & facet_appcx() {
                static FrameAppContext s_cx{
                    FrameAppConfig(Indentlog2Config::make_default(),
                                   FacetConfig::make_default())};

                return s_cx.cx<S_facet_tag>();
            }

            /** flywheel + the registrations a frame needs, once **/
            rp<AllocFlywheel> make_fw(const char * tag) {
                static bool s_once = []() {
                    REQUIRE(SetupObject2::register_facets());
                    SetupObject2::reflect_types();
                    return true;
                }();
                (void)s_once;

                ArenaConfig storage_cfg{ .name_ = ArenaNameStr::sprintf("%s.storage", tag),
                                         .size_ = 16*1024 };
                ArenaConfig strong_cfg { .name_ = ArenaNameStr::sprintf("%s.strong", tag),
                                         .size_ = 4*1024 };

                return AllocFlywheel::make_app(facet_appcx(), storage_cfg, strong_cfg);
            }
        }

        TEST_CASE("constructing-a-printjson-reflects-memorysizeinfo",
                  "[printjson][flywheel]")
        {
            /* MemorySizeInfo is the one part of a frame still rendered by
             * REFLECTION rather than by a bespoke printer -- it is already the
             * right shape, so restating its fields would be the duplication
             * the Info classes were retired for.
             *
             * Its registration moved into PrintJson's constructor on
             * 2026-09-22, from a free function a caller had to remember.  That
             * is what this pins: build one, and the description exists.
             */
            PrintJson print_json;

            auto td = Reflect::require<xo::mm::MemorySizeInfo>();

            REQUIRE(td->metatype() == Metatype::mt_struct);
            REQUIRE(td->n_child_fixed() == 7);

            /* the wire keys, asserted literally: "name", not "resource_name".
             * Members are named explicitly at registration so house style for
             * a c++ member cannot leak into the schema.
             *
             * detail_ is absent by CHOICE -- it points into the stack frame of
             * whoever ran the visit.  Raw pointers do reflect now
             * (.xo-backlog/xo-reflect/issues/01), so 7 rather than 8 is
             * curation, not a limitation.
             */
            REQUIRE(td->struct_member_name(0) == std::string("name"));
            REQUIRE(td->struct_member_name(5) == std::string("lo"));
            REQUIRE(td->struct_member_name(6) == std::string("hi"));
        } /*TEST_CASE(constructing-a-printjson-reflects-memorysizeinfo)*/

        TEST_CASE("empty-flywheel-renders-a-frame", "[printjson][flywheel]")
        {
            PrintJson print_json;
            rp<AllocFlywheel> fw = make_fw("utest.frame.empty");

            std::stringstream ss;
            print_json.print(*fw.get(), &ss);

            /* the whole frame, byte for byte.  Brittle on purpose: this IS the
             * wire contract, so any change to it should require someone to
             * look at what a consumer would now receive.  508 bytes for an
             * empty flywheel -- worth knowing before choosing a frame rate.
             *
             * Note "_name_": printjson tags every struct with its c++ type.
             * Useful to a consumer as a discriminator, and the reason a frame
             * is not as small as its field count suggests.
             */
            /* addresses are not reproducible under ASLR, so the frame cannot
             * be compared byte-for-byte as it stands.  Redact just those, and
             * compare the rest exactly -- the wire contract stays pinned, and
             * a changed key or a reordered field still fails.
             */
            const std::string frame
                = std::regex_replace(ss.str(),
                                     std::regex("\"(lo|hi)\": [0-9]+"),
                                     "\"$1\": ADDR");

            INFO("frame: " << frame);

            REQUIRE(frame == std::string(
                "{\"_name_\": \"Flywheel\""
                ", \"pools\": ["
                "{\"_name_\": \"MemorySizeInfo\""
                ", \"name\": \"utest.frame.empty.storage\""
                ", \"used\": 0, \"allocated\": 0, \"committed\": 0, \"reserved\": 16384"
                ", \"lo\": ADDR, \"hi\": ADDR}"
                ", {\"_name_\": \"MemorySizeInfo\""
                ", \"name\": \"utest.frame.empty.strong\""
                /* two pages, not one: DArenaVector::map inflates the request by
                 * the arena overhead, which pushes 4096 past a page boundary
                 */
                ", \"used\": 0, \"allocated\": 0, \"committed\": 0, \"reserved\": 8192"
                ", \"lo\": ADDR, \"hi\": ADDR}"
                ", {\"_name_\": \"MemorySizeInfo\""
                ", \"name\": \"utest.frame.empty.strong-free\""
                ", \"used\": 0, \"allocated\": 0, \"committed\": 0, \"reserved\": 4096"
                ", \"lo\": ADDR, \"hi\": ADDR}]"
                /* "RootSet", not "RootSetInfo", and "Flywheel" above rather
                 * than "FlywheelInfo": neither struct exists any more.  Both
                 * are now bespoke printers reading the live flywheel.  Every
                 * other key and value here is unchanged by that -- deliberately,
                 * since they are the wire contract
                 */
                ", \"strong\": {\"_name_\": \"RootSet\""
                /* 511, not 512: the per-allocation overhead costs one slot */
                ", \"size\": 0, \"capacity\": 511, \"live\": 0"
                ", \"free\": [], \"slots\": []}}"));

            /* the bound the redaction hid, asserted structurally instead.
             * Read through visit_pools, which is the path the printer takes
             */
            fw->visit_pools([](const xo::mm::MemorySizeInfo & pool) {
                    REQUIRE(static_cast<const char *>(pool.hi_)
                            - static_cast<const char *>(pool.lo_)
                            == static_cast<long>(pool.reserved_));
                });
        } /*TEST_CASE(empty-flywheel-renders-a-frame)*/

        TEST_CASE("occupied-slots-appear-in-the-frame", "[printjson][flywheel]")
        {
            using HFloat = DObjectHandle<APrintable, DFloat>;

            PrintJson print_json;
            rp<AllocFlywheel> fw = make_fw("utest.frame.live");
            auto alloc = with_facet<AAllocator>::mkobj(&fw->storage());

            auto h0 = HFloat::make_strong_ref
                (fw, with_facet<APrintable>::mkobj(DFloat::_box(alloc, 1.5)));
            auto h1 = HFloat::make_strong_ref
                (fw, with_facet<APrintable>::mkobj(DFloat::_box(alloc, 2.5)));

            std::stringstream ss;
            print_json.print(*fw.get(), &ss);

            const std::string frame = ss.str();
            INFO("frame: " << frame);

            /* substrings, not the whole frame: addresses and `used' are not
             * reproducible, so pinning them would make this a test of the
             * allocator's layout rather than of the schema
             */
            REQUIRE(frame.find("\"live\": 2") != std::string::npos);
            REQUIRE(frame.find("\"type\": \"xo::scm::DFloat\"") != std::string::npos);
            /* The first allocation sits at 16, not 0: an arena's storage now
             * begins behind a preamble (the DArena back pointer) AND each
             * allocation carries an 8-byte AllocHeader, which DHandleStore
             * requires on its storage arena (see its ctor).  Pinned because
             * the number is a consequence of both -- if this reads 0 again,
             * headers were silently turned off and alloc_info() became a
             * segfault waiting to happen.
             */
            REQUIRE(frame.find("\"offset\": 16") != std::string::npos);
            /* size is the allocation's own, read from its alloc header --
             * AllocInfo::size(), which the store-only ObjectSlot contract
             * makes safe to call (.xo-backlog/xo-facet/issues/04).  Spelled
             * with the enclosing "offset" so it cannot accidentally match
             * RootSet's unrelated "size" key one level up.
             *
             * 8 here, which happens to equal sizeof(DFloat) -- a fixed-size
             * representation needing no padding.  It is NOT read from the
             * type: DArray and DString fix capacity at construction rather
             * than in the type, so for those the two diverge and only the
             * header is right.  Observed (a first draft predicted 16, for the
             * allocation including its header; the header is excluded).
             */
            REQUIRE(frame.find("\"offset\": 16, \"size\": 8") != std::string::npos);
            /* no "ix" any more: every slot is emitted, so a consumer reads
             * an index from array position.  SlotInfo carried one until
             * 2026-09-20.
             */
            REQUIRE(frame.find("\"ix\":") == std::string::npos);

            /* the slots are ObjectSlot, so each carries its own _name_ */
            REQUIRE(frame.find("\"_name_\": \"ObjectSlot\"") != std::string::npos);

            /* offset is the frame's whole reason for existing -- it is how a
             * consumer follows one object across frames -- and it is resolved
             * per slot from its own pointer, so two slots must differ
             */
            std::vector<xo::facet::ObjectSlot> slot_v;
            fw->visit_object_slots([&slot_v](const xo::facet::ObjectSlot & slot) {
                    slot_v.push_back(slot);
                });

            REQUIRE(slot_v.size() == 2);
            REQUIRE(slot_v[0].opaque_data() != slot_v[1].opaque_data());

            REQUIRE(h0.object_ix() != h1.object_ix());
        } /*TEST_CASE(occupied-slots-appear-in-the-frame)*/

        TEST_CASE("released-slot-moves-to-the-free-list", "[printjson][flywheel]")
        {
            using HFloat = DObjectHandle<APrintable, DFloat>;

            PrintJson print_json;
            rp<AllocFlywheel> fw = make_fw("utest.frame.free");
            auto alloc = with_facet<AAllocator>::mkobj(&fw->storage());

            {
                auto h = HFloat::make_strong_ref
                    (fw, with_facet<APrintable>::mkobj(DFloat::_box(alloc, 1.5)));
                REQUIRE(h.object_ix() == 0);
            }

            std::stringstream ss;
            print_json.print(*fw.get(), &ss);

            const std::string frame = ss.str();
            INFO("frame: " << frame);

            /* the slot EMPTIES rather than disappearing: it stays at its
             * position and renders as null, its index is on `free', and `size'
             * stays 1 because it is a high-water mark.  A consumer animating
             * the root set draws a slot going dark, which it could not do if
             * the entry vanished and shifted everything after it.
             *
             * This is what retiring SlotInfo bought: an ObjectSlot can BE
             * empty, where a shadow struct had to be omitted.
             */
            REQUIRE(frame.find("\"size\": 1") != std::string::npos);
            REQUIRE(frame.find("\"live\": 0") != std::string::npos);
            REQUIRE(frame.find("\"free\": [0]") != std::string::npos);
            REQUIRE(frame.find("\"slots\": [null]") != std::string::npos);
        } /*TEST_CASE(released-slot-moves-to-the-free-list)*/

        TEST_CASE("free-list-order-is-reported-not-derived", "[printjson][flywheel]")
        {
            /* `free' is the one part of the frame that visit_object_slots
             * cannot supply.  The SET of free indices is derivable -- a
             * released slot renders as null -- but the ORDER is not, and the
             * order is what says which slot comes back next.  That is why
             * DHandleStore::visit_free_list exists alongside
             * visit_object_slots rather than the printer inferring it.
             *
             * Two releases, so an order exists to get wrong.
             */
            using HFloat = DObjectHandle<APrintable, DFloat>;

            PrintJson print_json;
            rp<AllocFlywheel> fw = make_fw("utest.frame.order");
            auto alloc = with_facet<AAllocator>::mkobj(&fw->storage());

            {
                auto h0 = HFloat::make_strong_ref
                    (fw, with_facet<APrintable>::mkobj(DFloat::_box(alloc, 1.5)));
                auto h1 = HFloat::make_strong_ref
                    (fw, with_facet<APrintable>::mkobj(DFloat::_box(alloc, 2.5)));

                REQUIRE(h0.object_ix() == 0);
                REQUIRE(h1.object_ix() == 1);
            }
            /* both released at scope exit, in REVERSE declaration order -- so
             * index 1 is released first and index 0 last
             */

            std::stringstream ss;
            print_json.print(*fw.get(), &ss);

            const std::string frame = ss.str();
            INFO("frame: " << frame);

            /* push order: 1 was released first.  Observed, and it is not the
             * order a reader would guess -- which is the point
             */
            REQUIRE(frame.find("\"free\": [1, 0]") != std::string::npos);
            REQUIRE(frame.find("\"slots\": [null, null]") != std::string::npos);

            /* and the LAST entry is the one that comes back -- so a consumer
             * reading `free' left-to-right is reading it backwards
             */
            auto h2 = HFloat::make_strong_ref
                (fw, with_facet<APrintable>::mkobj(DFloat::_box(alloc, 3.5)));

            REQUIRE(h2.object_ix() == 0);
        } /*TEST_CASE(free-list-order-is-reported-not-derived)*/

    } /*namespace ut*/
} /*namespace xo*/

/* end flywheel_frame.test.cpp */
