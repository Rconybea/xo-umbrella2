/** @file X1CollectorConfig.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "X1CollectorConfig.hpp"

namespace xo {
    namespace mm {

        X1CollectorConfig
        X1CollectorConfig::with_name(std::string name)
        {
            X1CollectorConfig copy = *this;
            copy.name_ = std::move(name);
            return copy;
        }

        X1CollectorConfig
        X1CollectorConfig::with_n_gen(std::uint32_t n_gen)
        {
            X1CollectorConfig copy = *this;
            copy.n_generation_ = n_gen;
            return copy;
        }

        X1CollectorConfig
        X1CollectorConfig::with_n_survive(std::uint32_t n_survive)
        {
            X1CollectorConfig copy = *this;
            copy.n_survive_threshold_ = n_survive;
            return copy;
        }

        X1CollectorConfig
        X1CollectorConfig::with_size(std::size_t gen_z)
        {
            X1CollectorConfig copy = *this;
            copy.arena_config_ = arena_config_.with_size(gen_z);
            return copy;
        }

        X1CollectorConfig
        X1CollectorConfig::with_debug_flag(bool x)
        {
            X1CollectorConfig copy = *this;
            copy.debug_flag_ = x;
            return copy;
        }

        X1CollectorConfig
        X1CollectorConfig::with_sanitize_flag(bool x)
        {
            X1CollectorConfig copy = *this;
            copy.sanitize_flag_ = x;
            return copy;
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end X1CollectorConfig.cpp */
