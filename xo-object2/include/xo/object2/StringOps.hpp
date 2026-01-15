/** @file StringOps.hpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "string/IGCObject_DString.hpp"
#include "DString.hpp"

namespace xo {
    namespace scm {
        /** @brief string functions
         *
         *  note: separate from DString
         **/
        struct StringOps {
            using AGCObject = xo::mm::AGCObject;
            using AAllocator = xo::mm::AAllocator;
            using size_type = DString::size_type;

            template <typename AFacet = AGCObject>
            static obj<AFacet,DString> empty(obj<AAllocator> mm,
                                             size_type cap);
        };

        template <typename AFacet>
        obj<AFacet,DString>
        StringOps::empty(obj<AAllocator> mm, size_type cap)
        {
            return obj<AFacet,DString>(DString::empty(mm, cap));
        }
    }
}

/* end StringOps.hpp */
