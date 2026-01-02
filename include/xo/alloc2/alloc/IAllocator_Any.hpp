/** @file IAllocator_Any.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "AAllocator.hpp"
#include "AllocIterator.hpp"
#include "typeseq.hpp"
#include <xo/facet/obj.hpp>

namespace xo {
    namespace mm { struct IAllocator_Any; }

    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AAllocator, DVariantPlaceholder> {
            using ImplType = xo::mm::IAllocator_Any;
        };
    }

    namespace mm {
        /** @class IAllocator_Any
         *  @brief Allocator implementation for empty variant instance.
         **/
        struct IAllocator_Any : public AAllocator {
            //using Impl = IAllocator_ImplType<xo::facet::DVariantPlaceholder>;
            using size_type = std::size_t;

            const AAllocator * iface() const { return std::launder(this); }

            // from AAllocator
            typeseq _typeseq() const noexcept  override { return s_typeseq; }

            // const methods
            [[noreturn]] std::string_view     name(Copaque) const noexcept override { _fatal(); }
            [[noreturn]] size_type        reserved(Copaque) const noexcept override { _fatal(); }
            [[noreturn]] size_type            size(Copaque) const noexcept override { _fatal(); }
            [[noreturn]] size_type       committed(Copaque) const noexcept override { _fatal(); }
            [[noreturn]] size_type       available(Copaque) const noexcept override { _fatal(); }
            [[noreturn]] size_type       allocated(Copaque) const noexcept override { _fatal(); }
            [[noreturn]] bool             contains(Copaque, const void *) const noexcept override { _fatal(); }
            [[noreturn]] AllocError     last_error(Copaque) const noexcept override { _fatal(); }
            [[noreturn]] AllocInfo      alloc_info(Copaque, value_type) const noexcept override { _fatal(); }
            // defn in .cpp - problematic to require compiler know vt<AAllocIterator> defn here
            //[[noreturn]] facet::vt<AAllocIterator> begin(Copaque, DArena &) const noexcept override; // { _fatal(); }
            [[noreturn]] range_type    alloc_range(Copaque, DArena &) const noexcept override { _fatal(); }

            // non-const methods
            [[noreturn]] bool               expand(Opaque, std::size_t) const noexcept override { _fatal(); }
            [[noreturn]] value_type          alloc(Opaque, std::size_t) const override { _fatal(); }
            [[noreturn]] value_type    super_alloc(Opaque, std::size_t) const override { _fatal(); }
            [[noreturn]] value_type      sub_alloc(Opaque, std::size_t, bool) const override { _fatal(); }
            [[noreturn]] void                clear(Opaque) const override { _fatal(); }
            [[noreturn]] void        destruct_data(Opaque) const override { _fatal(); }

        private:
            [[noreturn]] static void _fatal();

        public:
            static typeseq s_typeseq;
            static bool _valid;
        };
    }

}

/* end IAllocator_Any.hpp */
