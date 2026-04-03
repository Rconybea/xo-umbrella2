/** @file MutationLogConfig.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include "MutationLogConfig.hpp"

namespace xo {
    namespace mm {

        MutationLogConfig::MutationLogConfig(std::uint32_t ngen,
                                             std::uint32_t survive,
                                             std::size_t mlog_z,
                                             bool debug_flag)
            : n_generation_{ngen},
              n_survive_threshold_{survive},
              mutation_log_z_{mlog_z},
              debug_flag_{debug_flag}
        {}

    }
}

/* end MutationLogConfig.cpp */
