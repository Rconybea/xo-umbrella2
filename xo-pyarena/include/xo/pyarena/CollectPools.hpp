/** @file CollectPools.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include <xo/arena/MemorySizeInfo.hpp>
#include <vector>

namespace xo {
    namespace pyarena {
        /** @brief collect the pools @p src reports, as values python can keep.
         *
         *  For any @p src with a
         *    void visit_pools(const xo::mm::MemorySizeVisitor &) const
         *  method.
         *
         *  Returns the pools rather than taking a visitor, because python
         *  cannot honor the lifetime rule a visitor requires, namely:
         *  lifetime of MemorySizeInfo::detail_ may end once visit
         *  has completed.
         **/
        template <typename Src>
        std::vector<xo::mm::MemorySizeInfo> collect_pools(const Src & src) {
            std::vector<xo::mm::MemorySizeInfo> retval;

            src.visit_pools([&retval](const xo::mm::MemorySizeInfo & info) {
                retval.push_back(info);
                retval.back().detail_ = nullptr;
            });

            return retval;
        }

    } /*namespace pyarena*/
} /*namespace xo*/

/* end CollectPools.hpp */
