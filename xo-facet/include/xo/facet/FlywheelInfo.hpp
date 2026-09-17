/** @file FlywheelInfo.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include <xo/arena/MemorySizeInfo.hpp>
#include <string>
#include <vector>
#include <cstdint>

namespace xo::mm {
    /** @defgroup mm-flywheelinfo a flywheel's state, as a wire model
     *
     *  These types are a VIEW MODEL, not a description of
     *  @ref AllocFlywheel's layout.  The distinction is the whole design:
     *
     *  1. They are a SCHEMA, consumed by something outside this process (the
     *     intended first consumer is a browser animation, fed over a
     *     websocket).  A schema that tracked the c++ layout would turn every
     *     member rename into a change on the far side.
     *
     *  2. Reflecting the representation is not available for the parts that
     *     remain here.  @c strong_refs_ holds ERASED fops, whose reflection
     *     rotates through @c AReflectable and THROWS for a representation that
     *     has not opted in -- 2 of 56 had, as of 2026-09-15.  So a slot
     *     structurally cannot describe itself, and @ref SlotInfo is the only
     *     description available rather than a parallel one.
     *
     *     Where an existing type CAN be reflected, it is: pools are reported as
     *     @c MemorySizeInfo directly (2026-09-15), not copied into a shadow
     *     struct.  Only its @c detail_ is omitted -- a pointer into the
     *     visitor's stack frame, which must not be followed from a snapshot,
     *     and which is almost always null.  If the per-type histogram is
     *     wanted it belongs in its own report, at its own cadence.
     *
     *  3. @c xo-facet cannot depend on xo-printjson (`xo-printjson ->
     *     xo-printable2 -> xo-facet`), so the printer cannot live with the type
     *     the way SetupObject2's and SetupStringtable2's do.  Keeping these
     *     PODs free of reflection lets the registration live wherever the
     *     consumer does.
     *
     *  An earlier note here claimed AllocFlywheel could never reflect
     *  completely, because @c facet_appcx_ was a reference member and c++
     *  forbids forming a pointer-to-member of reference type.  That stopped
     *  being true when the appcx references became value-typed
     *  CreationEvidence.  So reflecting AllocFlywheel directly is now merely
     *  unattractive rather than impossible -- @ref FlywheelInfo earns its place
     *  as the frame ENVELOPE (where a sequence number would go), not because
     *  the alternative is blocked.  @ref SlotInfo's justification is untouched:
     *  an erased fop still cannot describe itself.
     *
     *  The cost is a second description of the same thing, which is a shape
     *  that has gone wrong repeatedly in this tree.  Mitigation:
     *  @ref DHandleStore::snapshot is written beside the members it reports,
     *  and pinned field-by-field by a test, so a new member that never reaches
     *  the wire fails rather than going quietly missing.
     **/
    ///@{

    /** one occupied slot of a root set.
     *
     *  Identity and location, no contents.  Everything here is available from
     *  @c ATop without rotating to any other facet, which is what makes it work
     *  for every representation rather than only the ones that have opted into
     *  @c AReflectable.
     *
     *  @c offset_ is the point of the whole structure for an animation: it
     *  locates the object within the flywheel's storage arena, so a consumer
     *  can draw it, and can show it MOVING when a collector relocates it.
     *  Reflecting the object's contents would not give that.
     *
     *  An OFFSET rather than an absolute address, for three reasons:
     *  - json numbers are IEEE754 doubles on the consumer's side, exact only
     *    below 2^53.  A 48-bit address has ~68x headroom but a 57-bit one
     *    (5-level paging) does not, and the rounding is undetectable there.
     *    An offset is bounded by the arena's extent, so the question does not
     *    arise.
     *  - it is the number a consumer actually wants; absolute would have it
     *    subtract @c lo from the pool record on every slot of every frame.
     *  - it survives remapping.  Absolute addresses all change when an arena
     *    is mapped somewhere else; offsets do not, so an object that did not
     *    move does not appear to.
     *
     *  NB no size field.  Per-allocation size comes from @c DArena::alloc_info,
     *  which reads an @c AllocHeader that is only written when
     *  @c ArenaConfig.store_header_flag_ is set -- so it is absent for a
     *  default arena, and a field that is usually missing is worse than none.
     **/
    struct SlotInfo {
        /** index in the root set's vector; matches @ref RootSetInfo::free_ **/
        std::uint32_t ix_ = 0;
        /** representation's type id, from ATop::_typeseq() **/
        std::int32_t typeseq_ = 0;
        /** representation's registered name; "" if never registered **/
        std::string type_;
        /** byte offset of the representation from the base of the flywheel's
         *  storage arena -- i.e. from @c pool_v_[0].lo_
         **/
        std::uint64_t offset_ = 0;
    };

    /** a root set's occupancy.
     *
     *  No name field: @ref FlywheelInfo::pool_v_ already names the arena
     *  backing this vector, and a second copy of a name is precisely the drift
     *  this file's header warns about.
     **/
    struct RootSetInfo {
        /** slots ever allocated.  A HIGH-WATER MARK: with a free list the
         *  vector never shrinks, so this is not the population
         **/
        std::uint32_t size_ = 0;
        /** slots the arena can hold **/
        std::uint32_t capacity_ = 0;
        /** occupied slots -- size_ minus free_.size() **/
        std::uint32_t live_ = 0;
        /** indices currently on the free list **/
        std::vector<std::uint32_t> free_;
        /** the occupied slots.  Cleared slots are omitted, so this has
         *  @ref live_ entries and the frame stays proportional to what is
         *  actually rooted
         **/
        std::vector<SlotInfo> slot_v_;
    };

    /** one frame: a flywheel's state at an instant **/
    struct FlywheelInfo {
        /** every pool this flywheel owns, in the order it reports them:
         *  the storage arena first, then the root set and its free list.
         *
         *  @c pool_v_[0] is the arena @ref SlotInfo::offset_ is relative to.
         **/
        std::vector<MemorySizeInfo> pool_v_;
        /** the strong root set.  Singular since 2026-09-13, when the weak set
         *  was retired (see .xo-backlog/pyobject2/issues/02); a vector here
         *  rather than a second member would invite it back
         **/
        RootSetInfo strong_;
    };

    ///@}
} /*namespace xo::mm*/

/* end FlywheelInfo.hpp */
