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
            /** @defgroup mm-allocinfo-traits **/
            ///@{

            using size_type = AllocHeader::size_type;
            using byte = std::byte;
            using span_type = std::pair<const byte *, const byte *>;

            ///@}

            /** @defgroup mm-allocinfo-ctors **/
            ///@{

            AllocInfo(const AllocHeaderConfig * p_cfg,
                      const byte * p_guard_lo,
                      const AllocHeader * p_hdr,
                      const byte * p_guard_hi) : p_config_{p_cfg},
                                                 p_guard_lo_{p_guard_lo},
                                                 p_header_{p_hdr},
                                                 p_guard_hi_{p_guard_hi} {}

            /** error when alloc-header not configured **/
            static AllocInfo error_not_configured(const AllocHeaderConfig * p_cfg) {
                return AllocInfo(p_cfg, nullptr, nullptr, nullptr);
            }
            /** error on deref empty iterator **/
            static AllocInfo error_invalid_iterator(const AllocHeaderConfig * p_cfg) {
                return AllocInfo(p_cfg, nullptr, nullptr, nullptr);
            }

            ///@}

            /** @defgroup mm-allocinfo-methods **/
            ///@{

            AllocHeader header() const noexcept { return *p_header_; }

            /** true for non-sentinel AllocInfo instance **/
            bool is_valid() const noexcept { return ((p_config_ != nullptr)
                                            && (p_header_ != nullptr)); }
            /** true iff sentinel tseq, flagging a forwarding pointer **/
            bool is_forwarding_tseq() const noexcept {
                return p_config_->is_forwarding_tseq(*p_header_);
            }

            /** Guard bytes preceding allocation-header **/
            span_type guard_lo() const noexcept;
            /** Type sequence number in garbage collector **/
            std::uint32_t tseq() const noexcept { return p_config_->tseq(*p_header_); }
            /** Allocation age in garbage collector **/
            std::uint32_t  age() const noexcept { return p_config_->age (*p_header_); }
            /** Allocation size (including allocator-supplied padding, excluding alloc header) **/
            size_type     size() const noexcept { return p_config_->size(*p_header_); }
            /** Payload for this allocation. This is the memory available to application **/
            span_type  payload() const noexcept;
            /** Guard bytes immediately following allocation **/
            span_type guard_hi() const noexcept;
            /** Number of guard bytes **/
            size_type  guard_z() const noexcept { return p_config_->guard_z_; }
            /** Value (fixed test pattern) of guard byte **/
            char guard_byte() const noexcept { return p_config_->guard_byte_; }

            ///@}

            /** @defgroup mm-allocinfo-instance-vars **/
            ///@{

            const AllocHeaderConfig * p_config_ = nullptr;
            const byte *            p_guard_lo_ = nullptr;
            const AllocHeader *       p_header_ = nullptr;
            const byte *            p_guard_hi_ = nullptr;

            ///@}
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end AllocInfo.hpp */
