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
#ifdef OBSOLETE // in GCObjectStore
                              std::uint32_t survive,
#endif
                              std::size_t mlog_z,
                              bool debug_flag);

#ifdef OBSOLETE
            /** generation that would contain an object that has survived
             *  @p age collections. Equals the number of times object
             *  has been promoted.
             *
             *  Must be consistent
             **/
            Generation age2gen(object_age age) const noexcept {
                return Generation(age % n_survive_threshold_);
            }
#endif

#ifdef OBSOLETE
            /** age threshold for promotion to generation @p g **/
            uint32_t promotion_threshold(Generation g) const noexcept {

                // TODO: may consider replacing with table-lookup
                // Require: if two distinct ages promote to some gen g at the same time,
                //          then they also promote to gen g+k at the same time for all k>0.

                return  g * n_survive_threshold_;
            }
#endif

        public:
            /** number of generations in use.
             *  Same as @ref X1CollectorConfig::n_generation_
             **/
            std::uint32_t n_generation_ = 0;

#ifdef OBSOLETE
            /** Number of promotion steps.
             *  An object that survives this number of collections
             *  advances to the next generation.
             **/
            uint32_t n_survive_threshold_ = 2;
#endif

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
