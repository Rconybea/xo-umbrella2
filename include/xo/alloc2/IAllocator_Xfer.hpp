/** @file IAllocator_Xfer.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "AAllocator.hpp"

namespace xo {
    namespace mm {
        /** @class IAllocator_Xfer
         *
         *  Adapts typed allocator implementation @tparam IAllocator_DRepr
         *  to type-erased @ref AAllocator interface
         **/
        template <typename DRepr, typename IAllocator_DRepr>
        struct IAllocator_Xfer : public AAllocator {
            // parallel interface to AAllocator, with specific data type
            using Impl = IAllocator_DRepr;
            using size_type = AAllocator::size_type;
            using value_type = AAllocator::value_type;

            static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
            static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

            // from AAllocator

            // const methods

            int32_t _typeseq() const noexcept override { return s_typeseq; }
            std::string_view    name(Copaque d) const noexcept override { return I::name(_dcast(d)); }
            size_type       reserved(Copaque d) const noexcept override { return I::reserved(_dcast(d)); }
            size_type           size(Copaque d) const noexcept override { return I::size(_dcast(d)); }
            size_type      committed(Copaque d) const noexcept override { return I::committed(_dcast(d)); }
            size_type      available(Copaque d) const noexcept override { return I::available(_dcast(d)); }
            size_type      allocated(Copaque d) const noexcept override { return I::allocated(_dcast(d)); }
            bool            contains(Copaque d, const void * p) const noexcept override {
                return I::contains(_dcast(d), p);
            }
            AllocatorError last_error(Copaque d) const noexcept override { return I::last_error(_dcast(d)); }

            // non-const methods

            bool              expand(Opaque d,
                                     std::size_t z) const noexcept override { return I::expand(_dcast(d), z); }
            value_type         alloc(Opaque d,
                                     std::size_t z) const override { return I::alloc(_dcast(d), z); }
            value_type   super_alloc(Opaque d,
                                     std::size_t z) const override { return I::super_alloc(_dcast(d), z); }
            value_type     sub_alloc(Opaque d,
                                     std::size_t z,
                                     bool complete_flag) const override {
                return I::sub_alloc(_dcast(d), z, complete_flag);
            }
            void               clear(Opaque d) const override { return I::clear(_dcast(d)); }
            void       destruct_data(Opaque d) const override { return I::destruct_data(_dcast(d)); }

        private:
            using I = Impl;

        public:
            static int32_t s_typeseq;
            static bool _valid;
        };

        template <typename DRepr, typename IAllocator_DRepr>
        int32_t
        IAllocator_Xfer<DRepr, IAllocator_DRepr>::s_typeseq = facet::typeseq::id<DRepr>();

        template <typename DRepr, typename IAllocator_DRepr>
        bool
        IAllocator_Xfer<DRepr, IAllocator_DRepr>::_valid = facet::valid_facet_implementation<AAllocator, IAllocator_Xfer>();
    } /*namespace mm*/
} /*namespace xo*/

/* end IAllocator_Xfer.hpp */
