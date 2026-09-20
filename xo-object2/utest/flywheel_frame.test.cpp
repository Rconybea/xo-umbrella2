/* @file flywheel_frame.test.cpp
 *
 * author: Roland Conybeare, Sep 2026
 *
 * JSON rendering of an AllocFlywheel's state -- one animation frame.
 *
 * NOT object2's subject.  AllocFlywheel is xo-facet's, and this lives here
 * only because xo-facet cannot reach a StructReflector; see
 * xo-object2/include/xo/object2/reflect_flywheel_info.hpp, which records why
 * and where it is expected to move.
 *
 * What is pinned here is a WIRE CONTRACT: the key names below are what a
 * consumer outside this process parses.  So they are asserted literally rather
 * than derived from the c++ member names -- a member rename must not silently
 * rename a key.
 *
 * Expectations are OBSERVED, never predicted.
 */

#include <xo/object2/reflect_flywheel_info.hpp>
#include <xo/object2/Float.hpp>   /* DFloat + IPrintable_DFloat */
#include <xo/object2/SetupObject2.hpp>
#include <xo/object2/number/IGCObject_DFloat.hpp>
#include <xo/facet/ObjectHandle.hpp>
#include <xo/facet/AllocFlywheel.hpp>
#include <xo/facet/FlywheelInfo.hpp>
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
    using xo::facet::FlywheelInfo;
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

        TEST_CASE("flywheel-info-reflects-as-a-struct", "[printjson][flywheel]")
        {
            xo::facet::reflect_flywheel_info();

            auto td = Reflect::require<FlywheelInfo>();

            REQUIRE(td->metatype() == Metatype::mt_struct);
            REQUIRE(td->n_child_fixed() == 2);

            /* the wire keys, asserted literally.  "pools" and not "pool_v":
             * reflect_flywheel_info names members explicitly so house style for
             * a vector member cannot leak into the schema
             */
            REQUIRE(td->struct_member_name(0) == std::string("pools"));
            REQUIRE(td->struct_member_name(1) == std::string("strong"));
        } /*TEST_CASE(flywheel-info-reflects-as-a-struct)*/

        TEST_CASE("empty-flywheel-renders-a-frame", "[printjson][flywheel]")
        {
            PrintJson print_json;
            xo::facet::reflect_flywheel_info();

            rp<AllocFlywheel> fw = make_fw("utest.frame.empty");

            std::stringstream ss;
            print_json.print(fw->snapshot(), &ss);

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
                "{\"_name_\": \"FlywheelInfo\""
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
                ", \"strong\": {\"_name_\": \"RootSetInfo\""
                /* 511, not 512: the per-allocation overhead costs one slot */
                ", \"size\": 0, \"capacity\": 511, \"live\": 0"
                ", \"free\": [], \"slots\": []}}"));

            /* the bound the redaction hid, asserted structurally instead */
            FlywheelInfo snap = fw->snapshot();
            for (const auto & pool : snap.pool_v_) {
                REQUIRE(static_cast<const char *>(pool.hi_)
                        - static_cast<const char *>(pool.lo_)
                        == static_cast<long>(pool.reserved_));
            }
        } /*TEST_CASE(empty-flywheel-renders-a-frame)*/

        TEST_CASE("occupied-slots-appear-in-the-frame", "[printjson][flywheel]")
        {
            using HFloat = DObjectHandle<APrintable, DFloat>;

            PrintJson print_json;
            xo::facet::reflect_flywheel_info();

            rp<AllocFlywheel> fw = make_fw("utest.frame.live");
            auto alloc = with_facet<AAllocator>::mkobj(&fw->storage());

            auto h0 = HFloat::make_strong_ref
                (fw, with_facet<APrintable>::mkobj(DFloat::_box(alloc, 1.5)));
            auto h1 = HFloat::make_strong_ref
                (fw, with_facet<APrintable>::mkobj(DFloat::_box(alloc, 2.5)));

            std::stringstream ss;
            print_json.print(fw->snapshot(), &ss);

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
            REQUIRE(frame.find("\"ix\": 0") != std::string::npos);
            REQUIRE(frame.find("\"ix\": 1") != std::string::npos);

            /* both slots carry a NON-ZERO address.  That field is the frame's
             * whole reason for existing -- it is how a consumer follows one
             * object across frames -- so a silently zero addr would make the
             * animation wrong while every other assertion still passed.
             */
            FlywheelInfo snap = fw->snapshot();
            REQUIRE(snap.strong_.slot_v_.size() == 2);
            REQUIRE(snap.strong_.slot_v_[0].offset_ == 16);  /* past the preamble + alloc header */
            REQUIRE(snap.strong_.slot_v_[1].offset_ != 0);
            REQUIRE(snap.strong_.slot_v_[0].offset_ != snap.strong_.slot_v_[1].offset_);

            REQUIRE(h0.object_ix() != h1.object_ix());
        } /*TEST_CASE(occupied-slots-appear-in-the-frame)*/

        TEST_CASE("released-slot-moves-to-the-free-list", "[printjson][flywheel]")
        {
            using HFloat = DObjectHandle<APrintable, DFloat>;

            PrintJson print_json;
            xo::facet::reflect_flywheel_info();

            rp<AllocFlywheel> fw = make_fw("utest.frame.free");
            auto alloc = with_facet<AAllocator>::mkobj(&fw->storage());

            {
                auto h = HFloat::make_strong_ref
                    (fw, with_facet<APrintable>::mkobj(DFloat::_box(alloc, 1.5)));
                REQUIRE(h.object_ix() == 0);
            }

            std::stringstream ss;
            print_json.print(fw->snapshot(), &ss);

            const std::string frame = ss.str();
            INFO("frame: " << frame);

            /* the slot is gone from `slots' and its index is on `free'; `size'
             * stays 1 because it is a high-water mark.  A consumer animating
             * the root set needs all three to draw a slot emptying rather than
             * disappearing.
             */
            REQUIRE(frame.find("\"size\": 1") != std::string::npos);
            REQUIRE(frame.find("\"live\": 0") != std::string::npos);
            REQUIRE(frame.find("\"free\": [0]") != std::string::npos);
            REQUIRE(frame.find("\"slots\": []") != std::string::npos);
        } /*TEST_CASE(released-slot-moves-to-the-free-list)*/

    } /*namespace ut*/
} /*namespace xo*/

/* end flywheel_frame.test.cpp */
