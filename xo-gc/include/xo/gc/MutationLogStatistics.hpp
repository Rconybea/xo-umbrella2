/** @file MutationLogStatistics.hpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

#include <cstddef>

namespace xo {
    namespace mm {

        /** @brief statistics from mlog forwarding **/
        class MutationLogStatistics {
        public:
            MutationLogStatistics() = default;

            MutationLogStatistics & operator+=(const MutationLogStatistics & x) {
                n_stale_ += x.n_stale_;
                n_live_parent_ += x.n_live_parent_;
                n_rescue_ += x.n_rescue_;
                n_triage_ += x.n_triage_;

                return *this;
            }

        public:
            /** count superseded mlog entries **/
            std::size_t n_stale_ = 0;
            /** count live parents encountered during mlog scan **/
            std::size_t n_live_parent_ = 0;
            /** count child subgraphs rescued during mlog scan **/
            std::size_t n_rescue_ = 0;
            /** count triaged mlog entries **/
            std::size_t n_triage_ = 0;
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end MutationLogStatistics.hpp */
