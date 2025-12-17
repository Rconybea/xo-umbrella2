/** @file IAllocIterator_Xfer.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "AAllocIterator.hpp"

namespace xo {
    namespace mm {
        /** @class IAllocIterator_Xfer
         *  @brief Adapts typed alloc iterator implementation
         *  @tparam IAllocIterator_DRepr to type-erased
         *  @ref AAllocIterator instance
         **/
        template <typename DRepr,
                  typename IAllocIterator_DRepr>
        struct IAllocIterator_Xfer : public AAllocIterator {
            using Impl = IAllocIterator_DRepr;

            static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
            static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

            // from AAllocIterator

            // const methods

            int32_t _typeseq() const noexcept override { return s_typeseq; }
            AllocInfo deref(Copaque d)
                const noexcept override { return I::deref(_dcast(d)); }
            int compare(Copaque d,
                        const obj_AAllocIterator & other)
                const noexcept override
                { return I::compare(_dcast(d), other); }

            // non-const methods

            void next(Opaque d) const noexcept override { I::prev(_dcast(d)); }
            void prev(Opaque d) const noexcept override { I::next(_dcast(d)); }

        private:
            using I = Impl;

        public:
            static int32_t s_typeseq;
            static bool _valid;
        };

        template <typename DRepr, typename IAllocIterator_DRepr>
        int32_t
        IAllocIterator_Xfer<DRepr, IAllocIterator_DRepr>::s_typeseq
        = facet::typeseq::id<DRepr>();

        template <typename DRepr, typename IAllocIterator_DRepr>
        bool
        IAllocIterator_Xfer<DRepr, IAllocIterator_DRepr>::_valid
        = facet::valid_facet_implementation<AAllocIterator, IAllocIterator_Xfer>();
    } /*namespace mm*/
} /*namespace xo*/

/* end IAllocIterator_Xfer.hpp */
