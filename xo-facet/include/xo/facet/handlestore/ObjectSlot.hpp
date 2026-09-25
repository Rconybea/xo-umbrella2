/** @file ObjectSlot.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "xo/facet/Top.hpp"

namespace xo::facet {

    template <typename Storage, typename Handle>
    class DHandleStore;

    /** @brief representation for an AllocFlywheel root pointer
     *
     *  Representation for a root object: an object that's
     *  accessible from python
     *
     *  @code
     *                                                                DArena
     *  +------------------+                       /----------------->+---+
     *  | ObjectHandleBase |  +-----------------+  |                  |   |
     *  | .memory        x--->| AllocFlywheel   |  |                  .   .
     *  | .object_ix       |  | .store        x----/                  .   .
     *  | .impl_handle   x |  |  .storage       |     ObjectSlot []   .   .
     *  +----------------|-+  |  .strong_refs x------>+---+           .   .
     *                   |    +-----------------+     |   |           .   .
     *                   |                            .   .           .   .
     *                   \--------------------------->| x------------>| X |
     *                                                .   .           | X |
     *                                                .   .           | X |
     *                                                |   |           .   .
     *                                                +---+           .   .
     *                                                                |   |
     *                                                                +---+
     *  @endcode
     *
     *  @par Provenance
     *
     *  A non-empty ObjectSlot can only be created by @ref DHandleStore.
     *  This allows several inferences:
     *  1. the object it refers to was allocated from some DHandleStore's
     *     storage arena A. Enforced by @ref DHandleStore::add_strong_ref
     *  2. A provides per-alloc headers.
     *  3. A has a non-zero base alignment
     *  4. That base alignemnt is common to all DHandleStore instances
     *  It follows that @ref DArena::alloc_info is safe for all ObjectSlot
     *  pointers, and @c DArena::obj2arena() can recovers A from
     *  object pointer alone.
     **/
    class ObjectSlot : public obj<xo::facet::ATop> {
    public:
        using ATop = xo::facet::ATop;

    public:
        /** the empty slot.  Refers to nothing, so anyone may make one **/
        ObjectSlot() = default;

    private:
        /** @p iface, @p data: an erased fop whose @p data was allocated from
         *  the storage arena of the DHandleStore performing the construction.
         *
         *  Private: see @ref ObjectSlot's Provenance note.  A caller wanting a
         *  slot goes through DHandleStore::add_strong_ref, which checks @p data
         *  against its own arena before reaching here.
         **/
        ObjectSlot(const ATop * iface, void * data) : obj<ATop>{iface, data} {}
        explicit ObjectSlot(const obj<ATop> & x) : obj<ATop>{x} {}

        template <typename Storage, typename Handle>
        friend class DHandleStore;
    };
} /*namespace xo::facet*/

/* end ObjectSlot.hpp */
