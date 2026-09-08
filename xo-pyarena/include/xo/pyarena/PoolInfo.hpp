/** @file PoolInfo.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include <xo/arena/MemorySizeInfo.hpp>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace xo {
    namespace pyarena {
        /** @brief owning snapshot of an xo::mm::MemorySizeInfo.
         *
         *  A MemorySizeInfo cannot survive the visit_pools() callback that
         *  produced it: DArena::visit_pools() assembles the histogram in a
         *  stack local and reports through a temporary, so both
         *  @c resource_name_ (a string_view) and @c detail_ (a bare pointer)
         *  dangle the moment the callback returns.
         *
         *  Python cannot honour that rule -- a callback is free to keep the
         *  object it was handed -- so the pybind layer converts on the way in
         *  and hands python one of these instead.  Everything here is owned.
         **/
        struct PoolInfo {
            using MemorySizeDetail = xo::mm::MemorySizeDetail;
            using MemorySizeInfo = xo::mm::MemorySizeInfo;

            /** copy everything reachable from @p info, while it is still alive **/
            static PoolInfo from(const MemorySizeInfo & info) {
                PoolInfo retval;

                retval.name_ = std::string(info.resource_name_);
                retval.used_ = info.used_;
                retval.allocated_ = info.allocated_;
                retval.committed_ = info.committed_;
                retval.reserved_ = info.reserved_;

                if (info.lo_)
                    retval.lo_ = reinterpret_cast<std::uintptr_t>(info.lo_);
                if (info.hi_)
                    retval.hi_ = reinterpret_cast<std::uintptr_t>(info.hi_);

                /* pools that don't keep a histogram report detail_=nullptr.
                 * skip empty slots: the array is fixed-size (32) and mostly
                 * sentinel, which would otherwise dominate the report.
                 */
                if (info.detail_) {
                    for (const auto & detail : *info.detail_) {
                        if (detail.n_alloc_)
                            retval.detail_.push_back(detail);
                    }
                }

                return retval;
            }

            /** name of the pool being reported **/
            std::string name_;
            /** bytes in use, excluding waste **/
            std::size_t used_ = 0;
            /** bytes allocated, including waste **/
            std::size_t allocated_ = 0;
            /** bytes backed by physical memory **/
            std::size_t committed_ = 0;
            /** address space obtained, whether or not committed **/
            std::size_t reserved_ = 0;
            /** start address, if the pool has one **/
            std::optional<std::uintptr_t> lo_;
            /** end address, if the pool has one **/
            std::optional<std::uintptr_t> hi_;
            /** per-type histogram; empty when the pool keeps none **/
            std::vector<MemorySizeDetail> detail_;
        };

        /** @brief collect the pools @p src reports, as owning snapshots.
         *
         *  For any @p src with a
         *    void visit_pools(const xo::mm::MemorySizeVisitor &) const
         *  method.  Materializing is not a cost the python binding can avoid
         *  (see PoolInfo above), so binding visit_pools() as a function that
         *  RETURNS the pools is strictly more convenient than one taking a
         *  callback -- python gets a list it can index, filter and keep.
         **/
        template <typename Src>
        std::vector<PoolInfo> collect_pools(const Src & src) {
            std::vector<PoolInfo> retval;

            src.visit_pools([&retval](const xo::mm::MemorySizeInfo & info) {
                retval.push_back(PoolInfo::from(info));
            });

            return retval;
        }

    } /*namespace pyarena*/
} /*namespace xo*/

/* end PoolInfo.hpp */
