/** @file MutationLogConfig.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include "MutationLogConfig.hpp"

namespace xo {
    namespace mm {

        MutationLogConfig::MutationLogConfig(std::uint32_t ngen,
                                             std::size_t mlog_z,
                                             bool enabled_flag,
                                             bool debug_flag)
            : n_generation_{ngen},
              mutation_log_z_{mlog_z},
              enabled_flag_{enabled_flag},
              debug_flag_{debug_flag}
        {}

    }
}

/* end MutationLogConfig.cpp */
