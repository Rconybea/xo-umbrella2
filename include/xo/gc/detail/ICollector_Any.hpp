/** @file ICollector_Any.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "ACollector.hpp"
#include "AGCObject.hpp"
//#include <cassert>

namespace xo {
    namespace mm { struct ICollector_Any; }

    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::ACollector, DVariantPlaceholder> {
            using ImplType = xo::mm::ICollector_Any;
        };
    }

    namespace mm {
        /** @class ICollector_Any
         *  @brief Stub Collector Implementation for empty variant instance
         **/
        struct ICollector_Any : public ACollector {
            using typeseq = xo::facet::typeseq;
            using size_type = std::size_t;

            // from ACollector
            typeseq _typeseq() const noexcept override { return s_typeseq; }

            // const methods
            [[noreturn]] size_type allocated(Copaque, generation, role) const noexcept override { _fatal(); }
            [[noreturn]] size_type reserved(Copaque, generation, role) const noexcept override { _fatal(); }
            [[noreturn]] size_type committed(Copaque, generation, role) const noexcept override { _fatal(); }

            // non-const methods
            [[noreturn]] bool install_type(Opaque, const AGCObject &) noexcept override { _fatal(); }
            [[noreturn]] void add_gc_root(Opaque, int32_t, Opaque *) override { _fatal(); }
            [[noreturn]] void forward_inplace(Opaque, AGCObject *, void **) override { _fatal(); }

        private:
            [[noreturn]] static void _fatal();

        public:
            static typeseq s_typeseq;
            static bool _valid;
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end ICollector_Any.hpp */
