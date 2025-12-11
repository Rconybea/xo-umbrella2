/** @file facet_rtti.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "facet.hpp"
#include <cstdint>

namespace xo {
    namespace facet {
        /** CRTP here
         *
         *  to be able to recover original type from a variant,
         *  we need some analog of dynamic_cast<>.
         *  We don't use c++ dynamic cast here because we
         *  are refusing to impose any restrictions on
         *  facet representation types, so in particular
         *  they will likely be of unrelated type.
         *
         *  Instead rely on the interface pointer to access
         *  information about the runtime type associated
         *  with a stored fat-object-pointer representation.
         **/
        template <typename AFacet>
        struct FacetRttiShim : public AFacet {
        };
    } /*namespace facet*/
} /*namespace xo*/

/* end facet_rtti.hpp */
