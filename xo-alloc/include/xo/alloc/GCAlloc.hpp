/* file GCAlloc.hpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#pragma once

namespace xo {
    namespace gc {
        class GC : public IAlloc {
            enum class Space { A, B, N_Space };
            enum class Gen { Nursery, Tenured };

        };

    } /*namespace mem */
} /*namespace xo*/


/* end GCAlloc.hpp */
