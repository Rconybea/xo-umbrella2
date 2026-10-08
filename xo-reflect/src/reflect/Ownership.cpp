/** @file Ownership.cpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "Ownership.hpp"

namespace xo {
    namespace reflect {
        const char *
        ownership_descr(Ownership x)
        {
            switch (x) {
            case Ownership::owning:   return "owning";
            case Ownership::shared:   return "shared";
            case Ownership::borrowed: return "borrowed";
            }

            return "???";
        } /*ownership_descr*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end Ownership.cpp */
