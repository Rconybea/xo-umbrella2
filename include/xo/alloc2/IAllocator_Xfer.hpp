/** @file IAllocator_Xfer.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "AAllocator.hpp"

namespace xo {
    namespace mm {
        /** @class IAllocator_Xfer
         **/
        template <typename DRepr, typename IAllocator_DRepr>
        struct IAllocator_Xfer : public AAllocator {
            // parallel interface to AAllocator, with specific data type
            using Impl = IAllocator_DRepr;

            static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }

            // from AAllocator
            int32_t _typeseq() const override { return s_typeseq; }
            const std::string & name(Copaque d) const override { return Impl::name(_dcast(d)); }
            std::size_t     reserved(Copaque d) const override { return Impl::reserved(*(DRepr*)d); }
            std::size_t         size(Copaque d) const override { return Impl::size(*(DRepr*)d); }
            std::size_t    committed(Copaque d) const override { return Impl::committed(*(DRepr*)d); }
            bool            contains(Copaque d, const void * p) const override { return Impl::contains(*(DRepr*)d, p); }

            std::byte *        alloc(Opaque d, std::size_t z) const override { return Impl::alloc(*(DRepr*)d, z); }
            void               clear(Opaque d) const override { return Impl::clear(*(DRepr*)d); }
            void       destruct_data(Opaque d) const override { return Impl::destruct_data(*(DRepr*)d); }

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
