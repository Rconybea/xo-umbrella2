/** @file IGCObject_Any.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "AGCObject.hpp"
#include <new>

namespace xo {
    namespace mm { struct IGCObject_Any; }

    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AGCObject, DVariantPlaceholder> {
            using ImplType = xo::mm::IGCObject_Any;
        };
    }

    namespace mm {
        /** @class IGCObject_Any
         *  @brief AGCObject implementation for empty variant instance
         **/
        struct IGCObject_Any : public AGCObject {
            using size_type = std::size_t;

            const AGCObject * iface() const { return std::launder(this); }

            // from AGCObject
            int32_t _typeseq() const noexcept override { return s_typeseq; }

            [[noreturn]] size_type shallow_size() const noexcept override { _fatal(); }
            [[noreturn]] Opaque * shallow_copy() const noexcept override { _fatal(); }
            [[noreturn]] size_type forward_children() const noexcept override { _fatal(); }

        private:
            [[noreturn]] static void _fatal();

        public:
            static int32_t s_typeseq;
            static bool _valid;
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end IGCObject_Any.hpp */
