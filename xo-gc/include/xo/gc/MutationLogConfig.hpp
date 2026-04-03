/** @file MutationLogConfig.hpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

#include "generation.hpp"
#include <cstddef>
#include <cstdint>

namespace xo {
    namespace mm {

        /** @brief configuration for MutationLogState **/
        class MutationLogConfig {
        public:
            MutationLogConfig(std::uint32_t ngen,
                              std::uint32_t survive,
                              std::size_t mlog_z,
                              bool debug_flag);

            /** age threshold for promotion to generation @p g **/
            uint32_t promotion_threshold(Generation g) const noexcept {

                // TODO: may consider replacing with table-lookup
                // Require: if two distinct ages promote to some gen g at the same time,
                //          then they also promote to gen g+k at the same time for all k>0.

                return  g * n_survive_threshold_;
            }


        public:
            /** number of generations in use.
             *  Same as @ref X1CollectorConfig::n_generation_
             **/
            std::uint32_t n_generation_ = 0;

            /** Number of promotion steps.
             *  An object that survives this number of collections
             *  advances to the next generation.
             **/
            uint32_t n_survive_threshold_ = 2;

            /** storage for xgen pointer bookkeeping (aka remembered sets).
             *  Use 3x this value per generation
             **/
            std::size_t mutation_log_z_ = 1024;

            /** true to enable debug logging **/
            bool debug_flag_ = false;
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end MutationLogConfig.hpp */
