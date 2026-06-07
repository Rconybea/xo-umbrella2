/** @file mmap_util.hpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "span.hpp"

namespace xo {
    namespace mm {
        struct mmap_util {
            using byte = std::byte;
            using span_type = span<byte>;
            using size_type = std::size_t;

            /** obtain uncommitted contiguous memory range comprising
             *  a whole multiple of @p align_z bytes, of at least size @p req_z,
             *  aligned on a @p align_z boundary.  Uncommitted memory is not (yet)
             *  backed by physical memory.
             *
             *  If @p enable_hugepage_flag is true and THP
             *  (transparent huge pages) are available, use THP for arena memory.
             *  This relieves TLB and page table memory when @p req_z is a lot larger than
             *  page size (likely 4KB).  Cost is that arena will consum physical memory in unit
             *  of @p align_z.  Arena may waste up to @p align_z bytes of memory as a result.
             *
             *  If @p enable_hugepage_flag is true, @p align_z should be huge page size
             *  (probably 2MB) for optimal performance.
             *
             *  At present the THP feature is not supported on OSX.
             *  May be supportable through mach_vm_allocate().
             *
             *  Note that we reject MAP_HUGETLB|MAP_HUGE_2MB flags to mmap here,
             *  since requires previously-reserved memory in /proc/sys/vm/nr_hugepages.
             *
             *  Write log messages iff @p debug_flag is true.
             *
             *  @return spqn giving reserved memory address range [lo,hi)
             **/
            static span_type map_aligned_range(size_type req_z,
                                               size_type align_z,
                                               bool enable_hugepage_flag,
                                               bool debug_flag);
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end mmap_util.hpp */
