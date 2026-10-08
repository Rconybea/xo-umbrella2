/** @file Ownership.hpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#pragma once

namespace xo {
    namespace reflect {
        /** @brief how a holder's lifetime relates to its children's **/
        enum class Ownership {
            /** holder owns the target. child lifetime ends with parent's **/
            owning,
            /** parent is one of several co-owners.
             *  child remains alive as long as this parent does.
             *  child lifetime ends with the last co-owner's.
             **/
            shared,
            /** parent does not own child.
             *  parent lifetime ending has no effect on child.
             **/
            borrowed,
        };

        const char * ownership_descr(Ownership x);
    } /*namespace reflect*/
} /*namespace xo*/

/* end Ownership.hpp */
