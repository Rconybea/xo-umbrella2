/** @file MutationLogConfig.hpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

#include <cstddef>
#include <cstdint>

namespace xo {
    namespace mm {

        class MutationLogConfig {
        public:
            MutationLogConfig(std::uint32_t ngen,
                              std::size_t mlog_z,
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

            /** true to enable debug logging **/
            bool debug_flag_ = false;
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end MutationLogConfig.hpp */
