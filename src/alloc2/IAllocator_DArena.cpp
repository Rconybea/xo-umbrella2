/** @file IAllocator_DArena.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "AllocIterator.hpp"
#include "arena/IAllocator_DArena.hpp"
#include "arena/IAllocIterator_DArenaIterator.hpp" // for alloc_range
#include "arena/DArenaIterator.hpp"
#include "padding.hpp"
#include <xo/facet/obj.hpp>
#include <xo/indentlog/scope.hpp>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <sys/mman.h>

namespace xo {
    using xo::facet::with_facet;
    using std::size_t;
    using std::byte;

    namespace mm {

        std::string_view
        IAllocator_DArena::name(const DArena & s) noexcept {
            return s.config_.name_;
        }

        size_t
        IAllocator_DArena::reserved(const DArena & s) noexcept {
            return s.reserved();
        }

        size_t
        IAllocator_DArena::size(const DArena & s) noexcept {
            return s.limit_ - s.lo_;
        }

        size_t
        IAllocator_DArena::committed(const DArena & s) noexcept {
            return s.committed();
        }

        size_t
        IAllocator_DArena::available(const DArena & s) noexcept {
            return s.available();
        }

        size_t
        IAllocator_DArena::allocated(const DArena & s) noexcept {
            return s.allocated();
        }

        bool
        IAllocator_DArena::contains(const DArena & s,
                                    const void * p) noexcept
        {
            return (s.lo_ <= p) && (p < s.hi_);
        }

        AllocError
        IAllocator_DArena::last_error(const DArena & s) noexcept {
            return s.last_error_;
        }

        AllocInfo
        IAllocator_DArena::alloc_info(const DArena & s, value_type mem) noexcept
        {
            return s.alloc_info(mem);
        }

        auto
        IAllocator_DArena::alloc_range(const DArena & s,
                                       DArena & ialloc) noexcept -> range_type
        {
            DArenaIterator * begin_ix = construct_with<DArenaIterator>(ialloc, &s, s.begin_header());
            DArenaIterator *   end_ix = construct_with<DArenaIterator>(ialloc, &s, s.end_header());

            obj<AAllocIterator> begin_obj = with_facet<AAllocIterator>::mkobj(begin_ix);
            obj<AAllocIterator>   end_obj = with_facet<AAllocIterator>::mkobj(  end_ix);

            return std::make_pair(begin_obj, end_obj);
        }

        bool
        IAllocator_DArena::expand(DArena & s, size_t target_z) noexcept
        {
            return s.expand(target_z);
        } /*expand*/

        std::byte *
        IAllocator_DArena::alloc(DArena & s,
                                 std::size_t req_z)
        {
            return s.alloc(req_z);
        }

        std::byte *
        IAllocator_DArena::super_alloc(DArena & s,
                                       std::size_t req_z)
        {
            return s.super_alloc(req_z);
        }

        std::byte *
        IAllocator_DArena::sub_alloc(DArena & s,
                                     std::size_t req_z,
                                     bool complete_flag)
        {
            return s.sub_alloc(req_z, complete_flag);
        }

        byte *
        IAllocator_DArena::_alloc(DArena & s,
                                  std::size_t req_z,
                                  DArena::alloc_mode mode)
        {
            return s._alloc(req_z, mode);
        }

        void
        IAllocator_DArena::clear(DArena & s)
        {
            s.clear();
            //s.checkpoint_ = s.lo_;
        }

        void
        IAllocator_DArena::destruct_data(DArena & s)
        {
            s.~DArena();
        }
    } /*namespace mm*/
} /*namespace xo*/

/* end IAllocator_DArena.cpp */
