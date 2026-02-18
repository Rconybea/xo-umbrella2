/** @file MemorySizeInfo.hpp
*
 *  @author Roland Conybeare, Feb 2026
 **/

#pragma once

#include <xo/reflectutil/typeseq.hpp>
#include <functional>
#include <string_view>
#include <cstddef>

namespace xo {
    namespace mm {

        struct MemorySizeDetail {
            using typeseq = xo::reflect::typeseq;

            /** identifies a c++ type T.  See xo/facet/TypeRegistry **/
            typeseq tseq_;
            /** number of T-instances **/
            uint32_t n_alloc_ = 0;
            /** bytes used by T-instances **/
            uint32_t z_alloc_ = 0;
        };

        struct MemorySizeInfo {
            using size_type = std::size_t;
            using DetailArrayType = std::array<MemorySizeDetail, 32>;

            MemorySizeInfo() = default;
            MemorySizeInfo(std::string_view name,
                           std::size_t u, std::size_t a, std::size_t c, std::size_t r,
                           DetailArrayType * detail)
                : resource_name_{name},
                  used_{u}, allocated_{a}, committed_{c}, reserved_{r}, detail_{detail}
            {}

            static MemorySizeInfo sentinel() { return MemorySizeInfo(); }

            /** resource name **/
            std::string_view resource_name_;
            /** memory used (excluding wasted space) **/
            std::size_t used_  = 0;
            /** memory allocated (including wasted space e.g. empty slots in hash tables **/
            std::size_t allocated_ = 0;
            /** memory committed (backed by physical memory) **/
            std::size_t committed_ = 0;
            /** memory reserved:
             *  virtual memory addresses range obtained, whether or not committed
             **/
            std::size_t reserved_ = 0;

            /** optional histogram with per-data-type counts **/
            DetailArrayType * detail_ = nullptr;
        };

        /** function that visits MemorySizeInfo for a collection of @p n memory pools.
         *  Each pool reported with index @p i in [0, n), with associated
         *  size record @p info.
         **/
        using MemorySizeVisitor = std::function<void (const MemorySizeInfo & info)>;
    }
}

/* end MemorySizeInfo.hpp */
