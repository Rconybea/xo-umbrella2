/** @file IAllocator_Any.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "AAllocator.hpp"
#include <cassert>

namespace xo {
    namespace mm {
        struct IAllocator_Any;
    }

    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AAllocator, DVariantPlaceholder> {
            using ImplType = xo::mm::IAllocator_Any;
        };
    }

    namespace mm {
        /** @class IAllocator_Any
         *  @brief Allocator implementation for variant instance.
         **/
        struct IAllocator_Any : public AAllocator {
            //using Impl = IAllocator_ImplType<xo::facet::DVariantPlaceholder>;
            using size_type = std::size_t;

            // from AAllocator
            int32_t _typeseq() const noexcept  override { return s_typeseq; }

            [[noreturn]] const std::string & name(Copaque) const noexcept override { _fatal(); }
            [[noreturn]] size_type       reserved(Copaque) const noexcept override { _fatal(); }
            [[noreturn]] size_type           size(Copaque) const noexcept override { _fatal(); }
            [[noreturn]] size_type      committed(Copaque) const noexcept override { _fatal(); }
            [[noreturn]] size_type      available(Copaque) const noexcept override { _fatal(); }
            [[noreturn]] size_type      allocated(Copaque) const noexcept override { _fatal(); }
            [[noreturn]] bool            contains(Copaque, const void *) const noexcept override { _fatal(); }

            [[noreturn]] bool              expand(Opaque, std::size_t) const noexcept override { _fatal(); }
            [[noreturn]] std::byte *        alloc(Opaque, std::size_t) const override { _fatal(); }
            [[noreturn]] void               clear(Opaque) const override { _fatal(); }
            [[noreturn]] void       destruct_data(Opaque) const override { _fatal(); }

        private:
            [[noreturn]] static void _fatal();

        public:
            static int32_t s_typeseq;
            static bool _valid;
        };
    }

}

/* end IAllocator_Any.hpp */
