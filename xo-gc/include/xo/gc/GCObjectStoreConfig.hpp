/** @file GCObjectStoreConfig.hpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

#include "Generation.hpp"
#include "object_age.hpp"
#include <xo/arena/DArena.hpp>

namespace xo {
    namespace mm {

        /** @brief record GCObjectStore configuration **/
        class GCObjectStoreConfig {
        public:
            GCObjectStoreConfig(const ArenaConfig & arena_cfg,
                                std::uint32_t ngen,
                                std::uint32_t nsurvive,
                                std::size_t object_types_z,
                                bool debug_flag);

            /** generation that would contain an object that has survived
             *  @p age collections. Equals the number of times object
             *  has been promoted.
             *
             *  Must be consistent with promotion_threshold(g)
             **/
            Generation age2gen(object_age age) const noexcept {
                return Generation(std::min(age / n_survive_threshold_,
                                           n_generation_ - 1));
            }

            /** age threshold for promotion to generation @p g **/
            uint32_t promotion_threshold(Generation g) const noexcept {

                // TODO: may consider replacing with table-lookup
                // Require: if two distinct ages promote to some gen g at the same time,
                //          then they also promote to gen g+k at the same time for all k>0.

                return  g * n_survive_threshold_;
            }


        public:
            /** Configuration for collector spaces.
             *  Will have (2 x G) of these,
             *  where G is @ref n_generation_.
             *  Not using name_ member.
             *
             *  REQUIRE:
             *  - arena_config_.store_header_flag_ must be true
             **/
            ArenaConfig arena_config_;

            /** number of generations in use.  Same as @ref X1CollectorConfig::n_generation_ **/
            std::uint32_t n_generation_ = 0;

            /** Number of promotion steps.
             *  An object that survives this number of collections
             *  advances to the next generation.
             **/
            std::uint32_t n_survive_threshold_ = 2;

            /** storage for N object types requires 8*N bytes **/
            std::size_t object_types_z_ = 2*1024*1024;

            /** true to enable debug logging **/
            bool debug_flag_ = false;
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end GCObjectStoreConfig.hpp */
