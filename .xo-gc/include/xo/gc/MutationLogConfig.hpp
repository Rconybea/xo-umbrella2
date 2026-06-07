/** @file MutationLogConfig.hpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

#include "object_age.hpp"
#include "Generation.hpp"
#include <cstddef>
#include <cstdint>

namespace xo {
    namespace mm {

        /** @brief configuration for MutationLogState **/
        class MutationLogConfig {
        public:
            MutationLogConfig(std::uint32_t ngen,
                              std::size_t mlog_z,
                              bool enabled_flag,
                              bool debug_flag);

        public:
            /** number of generations in use.
             *  Same as @ref X1CollectorConfig::n_generation_
             **/
            std::uint32_t n_generation_ = 0;

            /** storage for xgen pointer bookkeeping (aka remembered sets).
             *  Use 3x this value per generation
             **/
            std::size_t mutation_log_z_ = 1024;

            /** true if mlog feature enabled (i.e. incremental gc enabled).
             *  false to disable (in which case only full gc supported)
             **/
            bool enabled_flag_ = false;

            /** true to enable debug logging **/
            bool debug_flag_ = false;
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end MutationLogConfig.hpp */
