/* GcStateDescription.hpp */

#include "GcStateDescription.hpp"

GcStateDescription::GcStateDescription(const GcGenerationDescription & nursery,
                                       const GcGenerationDescription & tenured,
                                       std::size_t gc_size,
                                       std::size_t gc_committed,
                                       std::size_t gc_allocated,
                                       std::size_t gc_available,
                                       std::size_t gc_mlog_size,
                                       std::size_t total_promoted,
                                       std::size_t total_n_mutation)
    : gc_size_{gc_size},
      gc_committed_{gc_committed},
      gc_allocated_{gc_allocated},
      gc_available_{gc_available},
      gc_mlog_size_{gc_mlog_size},
      total_promoted_{total_promoted},
      total_n_mutation_{total_n_mutation}
{
    gen_state_v_[gen2int(generation::nursery)] = nursery;
    gen_state_v_[gen2int(generation::tenured)] = tenured;
}

/* end GcStateDescription.cpp */
