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
        struct IAllocator_Any : public AAllocator {
            //using Impl = IAllocator_ImplType<xo::facet::DVariantPlaceholder>;

            // from AAllocator
            int32_t _typeseq() const override { return s_typeseq; }

            const std::string & name(Copaque) const override { assert(false); static std::string * x; return *x; }
            std::size_t     reserved(Copaque) const override { assert(false); return 0ul; }
            std::size_t         size(Copaque) const override { assert(false); return 0ul; }
            std::size_t    committed(Copaque) const override { assert(false); return 0ul; }
            bool            contains(Copaque, const void *) const override { assert(false); return false; }

            std::byte *        alloc(Opaque, std::size_t) const override { assert(false); return nullptr; }
            void               clear(Opaque) const override { assert(false); }
            void       destruct_data(Opaque) const override { assert(false); }

            static int32_t s_typeseq;
            static bool _valid;
        };
    }

}

/* end IAllocator_Any.hpp */
