/** @file AAllocator.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "xo/facet/facet_implementation.hpp"
#include "xo/facet/typeseq.hpp"
#include <string>

namespace xo {
    namespace mm {
        using Copaque = const void *;
        using Opaque = void *;


        /** Abstract facet for allocation
         *
         *    <----------------------------size-------------------------->
         *    <------------committed-----------><-------uncommitted------>
         *    <--allocated-->
         *
         *    XXXXXXXXXXXXXXX___________________..........................
         *
         *    allocated:   in use
         *    committed:   physical memory obtained
         *    uncommitted: mapped in virtual memory, not backed by memory
         **/
        struct AAllocator {
            /** RTTI: unique id# for actual runtime data repr **/
            virtual int32_t _typeseq() = 0;
            /** optional name for this allocator.
             *  Labeling, for diagnostics.
             **/
            virtual const std::string & name(Copaque d) = 0;
            /** allocator size in bytes (up to reserved limit)
             *  includes allocated and uncomitted memory
             **/
            virtual std::size_t size(Copaque d) = 0;
            /** committed size (physical addresses obtained)
             **/
            virtual std::size_t committed(Copaque d) = 0;
            /** true iff pointer @p in range of this allocator
             **/
            virtual bool contains(Copaque d, const void * p) = 0;

            /** allocate @p z bytes of memory. **/
            virtual std::byte * alloc(Opaque d, std::size_t z) = 0;
            /** reset allocator to empty state **/
            virtual void clear(Opaque d) = 0;
            /** **/
            virtual void destruct_data(Opaque d) = 0;
        };

        template <typename DRepr>
        struct IAllocator_Impl;

        template <typename DRepr>
        struct IAllocator_Xfer : public AAllocator {
            // parallel interface to AAllocator, with specific data type
            using Impl = IAllocator_Impl<DRepr>;

            static const DRepr & _dcast(Copaque d) { return *(const DRepr *)d; }

            // from AAllocator
            int32_t _typeseq() override { return s_typeseq; }
            const std::string & name(Copaque d) override { return Impl::name(_dcast(d)); }
            std::size_t         size(Copaque d) override { return Impl::size(*(DRepr*)d); }
            std::size_t    committed(Copaque d) override { return Impl::committed(*(DRepr*)d); }
            bool            contains(Copaque d, const void * p) override { return Impl::contains(*(DRepr*)d, p); }

            std::byte *        alloc(Opaque d, std::size_t z) override { return Impl::alloc(*(DRepr*)d, z); }
            void               clear(Opaque d) override { return Impl::clear(*(DRepr*)d); }
            void       destruct_data(Opaque d) override { return Impl::destruct_data(*(DRepr*)d); }

            static int32_t s_typeseq;
            static bool _valid;
        };

        template <typename DRepr>
        int32_t
        IAllocator_Xfer<DRepr>::s_typeseq = facet::typeseq::id<DRepr>();

        template <typename DRepr>
        bool
        IAllocator_Xfer<DRepr>::_valid = facet::valid_facet_implementation<AAllocator, IAllocator_Xfer>();
    } /*namespace mm*/
} /*namespace xo*/

/* end AAllocator.hpp */
