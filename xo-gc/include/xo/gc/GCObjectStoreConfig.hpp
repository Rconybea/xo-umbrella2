/** @file GCObjectStoreConfig.hpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

#include <xo/arena/DArena.hpp>

namespace xo {
    namespace mm {

        /** @brief record GCObjectStore configuration **/
        class GCObjectStoreConfig {
        public:
            GCObjectStoreConfig(const ArenaConfig & arena_cfg,
                                std::uint32_t ngen,
                                bool debug_flag);

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
            /** true to enable debug logging **/
            bool debug_flag_ = false;
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end GCObjectStoreConfig.hpp */
