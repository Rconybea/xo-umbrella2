/** @file AllocInfo.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "AllocInfo.hpp"

namespace xo {
    namespace mm {
        auto
        AllocInfo::guard_lo() const noexcept -> span_type
        {
            if (!p_guard_lo_)
                return span_type(nullptr, nullptr);

            return span_type(p_guard_lo_,
                             p_guard_lo_ + p_config_->guard_z_);
        }

        auto
        AllocInfo::payload() const noexcept -> span_type
        {
            if (!p_header_)
                return span_type(nullptr, nullptr);

            byte * lo = (byte *)(p_header_ + 1);
            size_type z = this->size();

            return span_type(lo, lo+z);
        }

        auto
        AllocInfo::guard_hi() const noexcept -> span_type
        {
            if (!p_guard_hi_)
                return span_type(nullptr, nullptr);

            return span_type(p_guard_hi_,
                             p_guard_hi_ + p_config_->guard_z_);
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end AllocInfo.cpp */
