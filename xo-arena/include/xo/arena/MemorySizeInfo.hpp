/** @file MemorySizeInfo.hpp
*
 *  @author Roland Conybeare, Feb 2026
 **/

#pragma once

#include <xo/reflectutil/typeseq.hpp>
#include <xo/ppsink/Prettifier.hpp>
#include <cstddef>
#include <functional>
#include <string_view>

namespace xo {
    namespace pp { class PpSink; }

    namespace mm {

        struct MemorySizeDetail {
            using typeseq = xo::reflect::typeseq;
            using PpSink = xo::pp::PpSink;

            /** pretty print instance to @p sink **/
            void pretty(PpSink & sink) const;

            /** identifies a c++ type T.  See xo/facet/TypeRegistry **/
            typeseq tseq_;
            /** number of T-instances **/
            uint32_t n_alloc_ = 0;
            /** bytes used by T-instances **/
            uint32_t z_alloc_ = 0;
        };

        /** @brief Report memory consumption with arena-aware details
         **/
        struct MemorySizeInfo {
            using size_type = std::size_t;
            using PpSink = xo::pp::PpSink;
            using DetailArrayType = std::array<MemorySizeDetail, 32>;

            MemorySizeInfo() = default;
            MemorySizeInfo(std::string_view name,
                           std::size_t u, std::size_t a, std::size_t c, std::size_t r,
                           const void * lo, const void * hi,
                           DetailArrayType * detail)
                : resource_name_{name},
                  used_{u}, allocated_{a}, committed_{c}, reserved_{r}, lo_{lo}, hi_{hi}, detail_{detail}
            {}

            static MemorySizeInfo sentinel() { return MemorySizeInfo(); }

            /** number of populated entries in @ref detail_ (0 when absent) **/
            std::size_t n_detail() const noexcept;

            /** pretty print instance to @p sink **/
            void pretty(PpSink & sink) const;

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
            /** start address (optional) **/
            const void * lo_ = 0;
            /** end address (optional) **/
            const void * hi_ = 0;

            /** optional histogram with per-data-type counts **/
            DetailArrayType * detail_ = nullptr;
        };

        /** function that visits MemorySizeInfo for a collection of @p n memory pools.
         *  Each pool reported with index @p i in [0, n), with associated
         *  size record @p info.
         **/
        using MemorySizeVisitor = std::function<void (const MemorySizeInfo & info)>;
    } /*namespace mm*/

    namespace pp {
        /** pretty-print for MemorySizeDetail **/
        template <>
        struct Prettifier<xo::mm::MemorySizeDetail> {
            static void print(PpSink & sink, const xo::mm::MemorySizeDetail & x) {
                x.pretty(sink);
            }
        };

        /** pretty-print for MemorySizeInfo **/
        template <>
        struct Prettifier<xo::mm::MemorySizeInfo> {
            static void print(PpSink & sink, const xo::mm::MemorySizeInfo & x) {
                x.pretty(sink);
            }
        };
    } /*namespace pp*/
}

/* end MemorySizeInfo.hpp */
