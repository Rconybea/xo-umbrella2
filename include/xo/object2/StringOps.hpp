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

            /** wrapper for DString.empty() **/
            template <typename AFacet = AGCObject>
            static obj<AFacet,DString> empty(obj<AAllocator> mm,
                                             size_type cap);

            /** wrapper for DString.from_cstr() **/
            template <typename AFacet = AGCObject>
            static obj<AFacet,DString> from_cstr(obj<AAllocator> mm,
                                                 const char * cstr);

            /** wrapper for DString.clone() **/
            template <typename AFacet = AGCObject,
                      typename ASrcFacet = AGCObject>
            static obj<AFacet,DString> clone(obj<AAllocator> mm,
                                             obj<ASrcFacet,DString> src);
        };

        template <typename AFacet>
        obj<AFacet,DString>
        StringOps::empty(obj<AAllocator> mm, size_type cap)
        {
            return obj<AFacet,DString>(DString::empty(mm, cap));
        }

        template <typename AFacet>
        obj<AFacet,DString>
        StringOps::from_cstr(obj<AAllocator> mm, const char * cstr)
        {
            return obj<AFacet,DString>(DString::from_cstr(mm, cstr));
        }

        template <typename AFacet, typename ASrcFacet>
        obj<AFacet,DString>
        StringOps::clone(obj<AAllocator> mm, obj<ASrcFacet,DString> src)
        {
            return obj<AFacet,DString>(DString::clone(mm, src.data()));
        }
    }
}

/* end StringOps.hpp */
