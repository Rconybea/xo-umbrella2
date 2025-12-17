/** @file AllocInfo.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "AllocHeaderConfig.hpp"
#include <utility>

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
            using byte = std::byte;
            using span_type = std::pair<const byte *, const byte *>;

            AllocInfo(const AllocHeaderConfig * p_cfg,
                      const byte * p_guard_lo,
                      const AllocHeader * p_hdr,
                      const byte * p_guard_hi) : p_config_{p_cfg},
                                                 p_guard_lo_{p_guard_lo},
                                                 p_header_{p_hdr},
                                                 p_guard_hi_{p_guard_hi} {}

            /** error when alloc-header not configured **/
            static AllocInfo error_not_configured(AllocHeaderConfig * p_cfg) {
                return AllocInfo(p_cfg, nullptr, nullptr, nullptr);
            }

            /** true for non-sentinel AllocInfo instance **/
            bool is_valid() const { return (p_config_ != nullptr) && (p_header_ != nullptr); }

            /** Guard bytes preceding allocation-header **/
            span_type guard_lo() const noexcept {
                if (!p_guard_lo_)
                    return span_type(nullptr, nullptr);

                return span_type(p_guard_lo_,
                                 p_guard_lo_ + p_config_->guard_z_);
            }

            /** Type sequence number in garbage collector **/
            std::uint32_t tseq() const noexcept { return p_config_->tseq(*p_header_); }
            /** Allocation age in garbage collector **/
            std::uint32_t  age() const noexcept { return p_config_->age (*p_header_); }
            /** Allocation size (including allocator-supplied padding) **/
            size_type     size() const noexcept { return p_config_->size(*p_header_); }

            /** Guard bytes immediately following allocation **/
            span_type guard_hi() const noexcept {
                if (!p_guard_hi_)
                    return span_type(nullptr, nullptr);

                return span_type(p_guard_hi_,
                                 p_guard_hi_ + p_config_->guard_z_);
            }

            const AllocHeaderConfig * p_config_ = nullptr;
            const byte *            p_guard_lo_ = nullptr;
            const AllocHeader *       p_header_ = nullptr;
            const byte *            p_guard_hi_ = nullptr;
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end AllocInfo.hpp */
