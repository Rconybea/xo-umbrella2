/** @file ObjectSlot.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "xo/facet/Top.hpp"

namespace xo::facet {

    /** @brief representation for an AllocFlywhell root pointer
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
     **/
    class ObjectSlot : public obj<xo::facet::ATop> {
    public:
        using ATop = xo::facet::ATop;

    public:
        ObjectSlot() = default;
        ObjectSlot(const ATop * iface, void * data) : obj<ATop>{iface, data} {}
        explicit ObjectSlot(const obj<ATop> & x) : obj<ATop>{x} {}
    };
} /*namespace xo::facet*/

/* end ObjectSlot.hpp */
