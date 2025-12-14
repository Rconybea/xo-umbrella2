/** @file AGCObject.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "IAllocator_Any.hpp"
#include "RAllocator.hpp"
#include "xo/facet/facet_implementation.hpp"
#include "xo/facet/typeseq.hpp"
#include "xo/facet/obj.hpp"
#include <cstdint>
#include <cstddef>

namespace xo {
    namespace mm {
        using Copaque = const void *;
        using Opaque = void *;

        /** @class AObject
         *  @brief Abstract facet for collector-eligible data
         *
         *  Data that supports AGCObject can have memory managed
         *  by ACollector
         **/
        struct AGCObject {
            using size_type = std::size_t;

            /** RTTI: unique id# for actual runtime data representation **/
            virtual int32_t _typeseq() const noexcept = 0;

            virtual size_type shallow_size(Copaque d) const noexcept = 0;
            virtual Opaque * shallow_copy(Copaque d,
                                          obj<AAllocator> mm) const noexcept = 0;
            virtual size_type forward_children(Opaque d) const noexcept = 0;
        };

        // implementation IGCObject_DRepr of AGCObject for state DRepr
        // should provide a specialization:
        //
        //   template <>
        //   struct xo::facet::FacetImplementation<AGCObjectx, DRepr> {
        //       using ImplType = IGCObject_DRepr;
        //   };
        //
        //   then IGCObject_ImplType<DRepr> --> IGCObject_DRepr
        //
        template <typename DRepr>
        using IGCObject_ImplType = xo::facet::FacetImplType<AGCObject, DRepr>;
    } /*namespace mm*/
} /*namespace xo*/

/* end AGCObject.hpp */
