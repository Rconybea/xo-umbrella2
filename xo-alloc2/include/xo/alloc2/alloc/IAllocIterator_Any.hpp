/** @file IAllocIterator_Any.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "AAllocIterator.hpp"

namespace xo {
    namespace mm { struct IAllocIterator_Any; }

    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AAllocIterator, DVariantPlaceholder> {
            using ImplType = xo::mm::IAllocIterator_Any;
        };
    }

    namespace mm {
        /** @class IAllocIterator_Any
         *  @brief AllocIterator implementation for empty variant instance
         **/
        struct IAllocIterator_Any : public AAllocIterator {
            using typeseq = xo::facet::typeseq;

            const AAllocIterator * iface() const { return std::launder(this); }

            // from AAllocIterator
            typeseq _typeseq() const noexcept override { return s_typeseq; }

            // const methods
            [[noreturn]] AllocInfo deref(Copaque) const noexcept override { _fatal(); }
            [[noreturn]] cmpresult compare(Copaque,
                                           const obj_AAllocIterator &) const noexcept override { _fatal(); }

            // non-const methods
            [[noreturn]] void next(Opaque) const noexcept override { _fatal(); }

        private:
            [[noreturn]] static void _fatal();

        public:
            static typeseq s_typeseq;
            static bool _valid;
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end IAllocIterator_Any.hpp */
