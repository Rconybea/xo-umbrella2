/** @file MutationLogStore.hpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

#include "MutationLogConfig.hpp"
#include "X1CollectorConfig.hpp"
#include "GCObjectStore.hpp"
#include "MutationLogStatistics.hpp"
#include "MutationLogEntry.hpp"
#include <xo/arena/DArenaVector.hpp>
#include <array>

namespace xo {
    namespace mm {
        class DX1Collector;
        class X1VerifyStats;

        /** @brief container for X1 collector mutation logs
         **/
        class MutationLogStore {
        public:
            using MutationLog = DArenaVector<MutationLogEntry>;
            using size_type = DArena::size_type;

        public:
            explicit MutationLogStore(const MutationLogConfig & config,
                                      GCObjectStore * gco_store);

            /** Initialize mlog state
             *  with o/s page size @p page_z
             **/
            void init_mlogs(std::size_t page_z);

            /** total number of active mlog entries (across all generations)
             **/
            size_type mutation_log_entries() const noexcept;

            void visit_pools(const MemorySizeVisitor & visitor) const;

            /** verify consistent mlog state,
             *  on behalf of gc-aware object store @p gc.
             *  (using gc to identify location of objects).
             *  Update counters in @p *p_verify_stats.
             **/
            void verify_ok(GCObjectStore * gc,
                           X1VerifyStats * p_verify_stats) noexcept;

            /** on behalf of gc-aware object store @p gc,
             *  change the value of a child pointer at @p p_lhs
             *  with parent object @p parent.  p_lhs and parent must belong
             *  to the same allocation.
             **/
            void assign_member(GCObjectStore * gc,
                               void * parent,
                               obj<AGCObject> * p_lhs,
                               obj<AGCObject> rhs);

            /** swap {to, from} roles
             **/
            void swap_roles(Generation upto) noexcept;

            /** On behalf of collector @p gc:
             *
             *  forward mutation logs, for generations 0 <= g < @p upto,
             *  from from-space to to-space.
             **/
            void forward_mutation_log(DX1Collector * gc,
                                      Generation upto);

        private:
            /** aux init function: create mutation log **/
            MutationLog _make_mlog(uint32_t igen, char tag_char,
                                   size_t mlog_z, std::size_t page_z);

            /** Append a single mutation to log for generation @p dest_g
             *  Mutation modifies @p parent at address @p addr,
             *  to refer to @p rhs.
             *
             *  Require: mutation is from older->newer,
             *  see validation in DX1Collector::assign_member.
             *
             *  NOTE: rhs can probably be dropped. Initially thought
             *  helpful to keep wrapped obj version. On closer look
             *  not necessary. Important to remember that gc can't change
             *  any interface pointers, it strictly preserves them.
             *
             *  Since mutation log entries are specific to a particular
             *  rhs pointer value, they commit corresponding interface
             *  pointer.  This means can alway recover that pointer
             *  by consulting the AllocHeader for the pointer target
             */
            void _append_mutation(Generation dest_g,
                                  void * parent,
                                  void ** addr,
                                  obj<AGCObject> rhs);

            /** On behalf of collctor @p gc:
             *
             *  Perform one pass over contents of @p *from_mlog for generation @p gen.
             *  @p *from_mlog contains all {xgen,xage} pointers that target generation @p gen.
             *  Surviving mlog entries are moved to either @p *to_mlog or @p *triage_mlog,
             *  (generation < @p upto being collected this cycle).
             *
             *  Each mlog entry gets one of the following outcomes.
             *  1. skip.   mlog entry has been superseded by another mut at target site.
             *  2. keep.   mlog entry is live. destination has been evacuated,
             *             so source must be updated as well.
             *  3. triage. source of incoming object belongs to a generation that was collected,
             *             and has not been evacuated. Although appears to be garbage, it may
             *             be live after all if reachable from the destination of some other
             *             mlog entry in @p *to_mlog. Store these mlog entries in @p *triage_mlog.
             *
             *  @return number of mlog entries moved, whether to @p *to_mlog or @p *triage_mlog.
             **/
            MutationLogStatistics _forward_mutation_log_phase(obj<AGCObjectVisitor> gc,
                                                              Generation upto,
                                                              Generation gen,
                                                              MutationLog * from_mlog,
                                                              MutationLog * to_mlog,
                                                              MutationLog * triage_mlog);

            /** On behalf of collector @p gc:
             *
             *  During gc of generations g < @p upto,
             *  with a P->C edge represented by mlog entry @p from_entry,
             *  with parent P in generation @p parent_gen:
             *  ensure child C is evacuated, and append @p from_entry to
             *  @p keep_mlog.
             **/
            MutationLogStatistics _preserve_child_of_live_parent(obj<AGCObjectVisitor> gc,
                                                                 Generation upto,
                                                                 Generation parent_gen,
                                                                 const MutationLogEntry & from_entry,
                                                                 MutationLog * keep_mlog);

            /** On behalf of collector @p gc:
             *
             *  helper function to decide whether to keep a mutation log entry
             *  @return true iff mlog entry appended to @p keep_mlog
             **/
            bool _check_keep_mutation_aux(const MutationLogEntry & from_entry,
                                          Generation parent_gen_to,
                                          void * child_to,
                                          MutationLog * keep_mlog);


        public:
            /** configuration for mlog store **/
            MutationLogConfig config_;

            /** stores GCOs (gc-aware objects) owned by the incremental collector
             *  with this mutaiton-log store
             **/
            GCObjectStore * gco_store_ = nullptr;

            /** Cross-generational mutations tracked in MutationLogs.
             *  We need three logs per generation:
             *  A. one to observe and remember mutations in to-space
             *     during normal operation (between GC cycles)
             *  B. during GC: 2nd mlog to hold entries from from-mlog
             *     that will still be needed post-GC (because ptr direction
             *     from higher gen to lower gen after cycle).
             *  C. during GC: 3rd mlog to triage entries for which
             *     liveness of pointer source isn't yet established.
             *
             * NOTE: indexed on generation of pointer *destination*
             **/
            std::array<MutationLog, c_max_generation - 1> mlog_storage_[c_n_role + 1];

            /** mlog pointers.  The roles of mlog_storage_[*][g] get permuted
             *  as each collection cycle proceeds
             **/
            std::array<MutationLog *, c_max_generation - 1> mlog_[c_n_role + 1];
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end MutationLogStore.hpp */
