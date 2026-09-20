/** @file FlywheelInfo.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "handlestore/ObjectSlot.hpp"
#include <xo/arena/MemorySizeInfo.hpp>
#include <string>
#include <vector>
#include <cstdint>

namespace xo::facet {
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
     *     has not opted in.
     *
     *     A slot used to need a shadow struct for that reason.  It no longer
     *     does: @c ObjectSlot derives from @c obj<ATop> rather than aliasing
     *     it, so it escapes FopTdx's erased path and reflects as an atom, and
     *     @c JsonPrinter_ObjectSlot renders it directly.  @ref slot_v_ holds
     *     the slots themselves.
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
     *  the alternative is blocked.  The slots themselves no longer need a view
     *  model at all -- see @ref RootSetInfo::slot_v_.
     *
     *  The cost is a second description of the same thing, which is a shape
     *  that has gone wrong repeatedly in this tree.  Mitigation:
     *  @ref DHandleStore::snapshot is written beside the members it reports,
     *  and pinned field-by-field by a test, so a new member that never reaches
     *  the wire fails rather than going quietly missing.
     **/
    ///@{

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
        /** every slot, INCLUDING the cleared ones -- so a slot's index is its
         *  position here, and @ref free_ indexes into it directly.
         *
         *  The slots are @c ObjectSlot, not a shadow struct.  That was
         *  @c SlotInfo until 2026-09-20, which existed because an erased fop
         *  could not describe itself; @c JsonPrinter_ObjectSlot now does, and
         *  resolves each slot's offset from its own pointer via
         *  @c DArena::obj2arena rather than needing a base handed in.  A
         *  cleared slot renders as @c null.
         **/
        std::vector<ObjectSlot> slot_v_;
    };

    /** one frame: a flywheel's state at an instant **/
    struct FlywheelInfo {
        /** every pool this flywheel owns, in the order it reports them:
         *  the storage arena first, then the root set and its free list.
         *
         *  @c pool_v_[0] is the arena a slot's rendered offset is relative to;
         *  JsonPrinter_ObjectSlot recovers it per slot via DArena::obj2arena.
         **/
        std::vector<xo::mm::MemorySizeInfo> pool_v_;
        /** the strong root set.  Singular since 2026-09-13, when the weak set
         *  was retired (see .xo-backlog/pyobject2/issues/02); a vector here
         *  rather than a second member would invite it back
         **/
        RootSetInfo strong_;
    };

    ///@}
} /*namespace xo::facet*/

/* end FlywheelInfo.hpp */
