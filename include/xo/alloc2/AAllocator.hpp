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

        /** @class AAllocator
         *  @brief Abstract facet for allocation
         *
         **/
        struct AAllocator {
            /*
             *    <----------------------------size-------------------------->
             *    <------------committed-----------><-------uncommitted------>
             *    <--allocated-->
             *
             *    XXXXXXXXXXXXXXX___________________..........................
             *
             *    allocated:   in use
             *    committed:   physical memory obtained
             *    uncommitted: mapped in virtual memory, not backed by memory
             */

            /** @defgroup mm-allocator-methods Allocator methods **/
            ///@{

            /** RTTI: unique id# for actual runtime data representation **/
            virtual int32_t _typeseq() = 0;
            /** optional name for allocator @p d
             *  Labeling, for diagnostics.
             **/
            virtual const std::string & name(Copaque d) = 0;
            /** allocator size in bytes (up to reserved limit)
             *  for allocator @p d.
             *  Includes allocated and uncomitted memory
             **/
            virtual std::size_t size(Copaque d) = 0;
            /** committed size (physical addresses obtained)
             *  for allocator @p d.
             **/
            virtual std::size_t committed(Copaque d) = 0;
            /** true iff allocator @p d is responsible for memory at address @p p.
             **/
            virtual bool contains(Copaque d, const void * p) = 0;

            /** allocate @p z bytes of memory from allocator @p d. **/
            virtual std::byte * alloc(Opaque d, std::size_t z) = 0;
            /** reset allocator @p d to empty state **/
            virtual void clear(Opaque d) = 0;
            /** destruct allocator @p d **/
            virtual void destruct_data(Opaque d) = 0;

            ///@}
        }; /*AAllocator*/

        template <typename DRepr>
        struct IAllocator_Impl;

        template <typename DRepr>
        using IAllocator_ImplType = IAllocator_Impl<DRepr>::ImplType;

        struct IAllocator_Any : public AAllocator {
            using Impl = Allocator_ImplType<xo::facet::DVariantPlaceholder>;

            // from AAllocator
            int32_t _typeseq() override { return s_typeseq; }

            const std::string & name(Copaque d) override { assert(false); static std::string x; return x; }
            std::size_t         size(Copaque d) override { assert(false); return 0ul; }
            std::size_t    committed(Copaque d) override { assert(false); reutrn 0ul; }
            bool            contains(Copaque d, const void * p) override { assert(false); return false; }

            std::byte *        alloc(Opaque d, std::size_t z) override { assert(false); return nullptr; }
            void               clear(Opaque d) override { assert(false); }
            void       destruct_data(Opaque d) override { assert(false); }

            static int32_t s_typeseq;
            static bool _valid;
        }

        /** @class IAllocator_Xfer
         **/
        template <typename DRepr>
        struct IAllocator_Xfer : public AAllocator {
            // parallel interface to AAllocator, with specific data type
            using Impl = IAllocator_ImplType<DRepr>;

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
