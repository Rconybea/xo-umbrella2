/** @file FlywheelInfo.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "handlestore/ObjectSlot.hpp"
#include "handlestore/DHandleStore.hpp"
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
     *  2. Only ONE member survives here, and it is a VIEW rather than a copy.
     *     The root set used to be copied into a @c RootSetInfo -- five fields
     *     and a vector of slots, restated in three places (the struct, the
     *     copier, the reflection).  It is now @ref strong_, a pointer to the
     *     live store, rendered by @c JsonPrinter_RootSet walking
     *     @c DHandleStore::visit_object_slots and @c visit_free_list.
     *
     *     **So a frame is not a snapshot of the root set.**  It reads the
     *     store at the moment it is PRINTED, not the moment
     *     @c AllocFlywheel::snapshot ran, and it dangles if the flywheel
     *     outlives neither.  That is inherent in not copying, and it suits the
     *     intended use -- take a frame, serialise it, discard it -- but a
     *     caller that holds a frame across a mutation gets the later state.
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
     *  the alternative is blocked.
     *
     *  The cost used to be a second description of the same thing, which is a
     *  shape that has gone wrong repeatedly in this tree.  Retiring
     *  @c RootSetInfo removed that copy; what remains is one reflected member
     *  (@ref pool_v_) and one printed by hand.
     **/
    ///@{


    /** one frame: a flywheel's state at an instant **/
    struct FlywheelInfo {
        /** every pool this flywheel owns, in the order it reports them:
         *  the storage arena first, then the root set and its free list.
         *
         *  @c pool_v_[0] is the arena a slot's rendered offset is relative to;
         *  JsonPrinter_ObjectSlot recovers it per slot via DArena::obj2arena.
         **/
        std::vector<xo::mm::MemorySizeInfo> pool_v_;
        /** the strong root set, BORROWED -- see point 2 of this group's
         *  header for what that costs.
         *
         *  Rendered by @c JsonPrinter_RootSet, which reflection could not do:
         *  a raw pointer has no EstablishTdx specialisation, so this reflects
         *  as an atom and the printer keys on the POINTER type rather than
         *  taking @c print_generic_pointer's dispatch to a pointee.  Null
         *  renders as @c null.
         *
         *  Singular since 2026-09-13, when the weak set was retired (see
         *  .xo-backlog/pyobject2/issues/02); a vector here rather than a
         *  second member would invite it back.
         **/
        const DHandleArena<ObjectSlot> * strong_ = nullptr;
    };

    ///@}
} /*namespace xo::facet*/

/* end FlywheelInfo.hpp */
