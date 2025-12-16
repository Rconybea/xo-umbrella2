/** @file AllocInfo.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "AllocHeader.hpp"

namespace xo {
    namespace mm {
        /** @class AllocInfo
         *  @brief bookkeeping information for an allocation
         *
         *  AllocInfo instances are 1:1 with sum of calls to
         *  {@ref AAllocator::alloc, @ref AAllocator::alloc_super}
         *
         **/
        struct AllocInfo {
            using size_type = AllocHeader::size_type;

            AllocInfo(const AllocHeaderConfig * p_cfg, const AllocHeader * p_hdr)
                : p_config_{p_cfg}, p_header_{p_hdr} {}

            /** error when alloc-header not configured **/
            static AllocInfo error_not_configured(AllocHeaderConfig * p_cfg) { return AllocInfo(p_cfg, nullptr); }

            /** true for non-sentinel AllocInfo instance **/
            bool is_valid() const { return (p_config_ != nullptr) && (p_header_ != nullptr); }

            /** Type sequence number in garbage collector **/
            std::uint32_t tseq() const noexcept { return p_config_->tseq(*p_header_); }
            /** Allocation age in garbage collector **/
            std::uint32_t  age() const noexcept { return p_config_->age (*p_header_); }
            /** Allocation size (including allocator-supplied padding) **/
            size_type     size() const noexcept { return p_config_->size(*p_header_); }

            const AllocHeaderConfig * p_config_ = nullptr;
            const AllocHeader *       p_header_ = nullptr;
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end AllocInfo.hpp */
