/** @file RCollector_Any.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "ACollector.hpp"
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
            using size_type = std::size_t;

            // from ACollector
            int32_t _typeseq() const noexcept override { return s_typeseq; }

            // const methods
            [[noreturn]] size_type allocated(Copaque, generation, role) const noexcept { _fatal(); }
            [[noreturn]] size_type reserved(Copaque, generation, role) const noexcept { _fatal(); }
            [[noreturn]] size_type committed(Copaque, generation, role) const noexcept { _fatal(); }

            // non-const methods
            [[noreturn]] void install_type(Opaque, int32_t, IGCObject_Any &) noexcept { _fatal(); }
            [[noreturn]] void add_gc_root(Opaque, int32_t, Opaque *) { _fatal(); }
            [[noreturn]] void forward_inplace(Opaque, Opaque **) { _fatail(); }

        private:
            [[noreturn]] static void _fatal();

        public:
            static int32_t s_typeseq;
            static bool _valid;
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end RCollector_Any.hpp */
