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
         *  @tparam DRepr target representation
         *  @tparam IAllocator_DRepr typed interface for @p DRepr
         *
         *  Adapts typed allocator implementation @p IAllocator_DRepr
         *  to type-erased @ref AAllocator interface
         **/
        template <typename DRepr, typename IAllocator_DRepr>
        struct IAllocator_Xfer : public AAllocator {
            /** @defgroup mm-allocator-xfer-types **/
            ///@{
            // parallel interface to AAllocator, with specific data type
            using Impl = IAllocator_DRepr;
            using size_type = AAllocator::size_type;
            using value_type = AAllocator::value_type;
            using typeseq = AAllocator::typeseq;
            ///@}

            /** @defgroup mm-allocator-xfer-methods IAllocator_Xfer methods **/
            ///@{

            static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }
            static DRepr & _dcast(Opaque d) { return *(DRepr *)d; }

            // from AAllocator

            // const methods

            /** return typeseq for @tparam DRepr **/
            typeseq _typeseq() const noexcept override { return s_typeseq; }
            std::string_view    name(Copaque d) const noexcept override { return I::name(_dcast(d)); }
            size_type       reserved(Copaque d) const noexcept override { return I::reserved(_dcast(d)); }
            size_type           size(Copaque d) const noexcept override { return I::size(_dcast(d)); }
            size_type      committed(Copaque d) const noexcept override { return I::committed(_dcast(d)); }
            size_type      available(Copaque d) const noexcept override { return I::available(_dcast(d)); }
            size_type      allocated(Copaque d) const noexcept override { return I::allocated(_dcast(d)); }
            bool            contains(Copaque d, const void * p) const noexcept override {
                return I::contains(_dcast(d), p);
            }
            AllocError    last_error(Copaque d) const noexcept override { return I::last_error(_dcast(d)); }
            AllocInfo     alloc_info(Copaque d, value_type mem) const noexcept override {
                return I::alloc_info(_dcast(d), mem);
            }
            range_type   alloc_range(Copaque d, DArena & mm) const noexcept override { return I::alloc_range(_dcast(d), mm); }

            // non-const methods

            bool              expand(Opaque d,
                                     std::size_t z) const noexcept override { return I::expand(_dcast(d), z); }
            value_type         alloc(Opaque d,
                                     typeseq t,
                                     std::size_t z) const override { return I::alloc(_dcast(d), t, z); }
            value_type   super_alloc(Opaque d,
                                     typeseq t,
                                     std::size_t z) const override { return I::super_alloc(_dcast(d), t, z); }
            value_type     sub_alloc(Opaque d,
                                     std::size_t z,
                                     bool complete_flag) const override {
                return I::sub_alloc(_dcast(d), z, complete_flag);
            }
            value_type    alloc_copy(Opaque d,
                                     value_type src) const override { return I::alloc_copy(_dcast(d), src); }
            void               clear(Opaque d) const override { return I::clear(_dcast(d)); }
            void       destruct_data(Opaque d) const override { return I::destruct_data(_dcast(d)); }
            ///@}

        private:
            using I = Impl;

        public:
            static xo::facet::typeseq s_typeseq;
            static bool _valid;
        };

        template <typename DRepr, typename IAllocator_DRepr>
        xo::facet::typeseq
        IAllocator_Xfer<DRepr, IAllocator_DRepr>::s_typeseq = facet::typeseq::id<DRepr>();

        template <typename DRepr, typename IAllocator_DRepr>
        bool
        IAllocator_Xfer<DRepr, IAllocator_DRepr>::_valid = facet::valid_facet_implementation<AAllocator, IAllocator_Xfer>();
    } /*namespace mm*/
} /*namespace xo*/

/* end IAllocator_Xfer.hpp */
