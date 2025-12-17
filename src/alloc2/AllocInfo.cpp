/** @file AllocInfo.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "alloc/AllocInfo.hpp"

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
