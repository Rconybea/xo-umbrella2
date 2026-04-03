/** @file MutationLogState.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include "MutationLogState.hpp"
#include "DX1Collector.hpp"

namespace xo {
    namespace mm {

        MutationLogState::MutationLogState(const MutationLogConfig & config)
            : config_{config}
        {}

        void
        MutationLogState::init_mlogs(std::size_t page_z)
        {
            assert(c_n_role + 1 == 3);

            for (uint32_t igen = 0, ngen = config_.n_generation_; igen + 1 < ngen; ++igen) {
                // special case: no use for mutation log for youngest generation,
                // so don't trouble to allocate one

                if (igen + 1 < c_max_generation) {
                    std::array<char, 3> label_v{'a', 'b', 'c'};

                    for (std::uint32_t mlog_role = 0; mlog_role < c_n_role + 1; ++mlog_role) {
                        this->mlog_storage_[mlog_role][igen]
                            = _make_mlog(igen,
                                         label_v[mlog_role],
                                         config_.mutation_log_z_,
                                         page_z);

                        this->mlog_[mlog_role][igen]
                            = &(mlog_storage_[mlog_role][igen]);
                    }
                } else {
                    assert(false);
                }
            }

            if (config_.n_generation_ > 0) {
                for (std::uint32_t igen = config_.n_generation_ - 1;
                     igen + 1 < c_max_generation; ++igen) {

                    for (std::uint32_t mlog_role = 0; mlog_role < c_n_role + 1; ++mlog_role)
                        this->mlog_[mlog_role][igen] = nullptr;
                }
            } else {
                assert(false);
            }
        }

        auto
        MutationLogState::_make_mlog(uint32_t igen, char tag_char,
                                     size_t mlog_z, size_t page_z) -> MutationLog
        {
            char buf[40];
            snprintf(buf, sizeof(buf), "x1-mlog-G%u-%c", igen, tag_char);

            return MutationLog::map(ArenaConfig{.name_ = std::string(buf),
                                                .size_ = mlog_z,
                                                .hugepage_z_ = page_z,
                                                .store_header_flag_ = false});
        }

        auto
        MutationLogState::mutation_log_entries() const noexcept -> size_type
        {
            size_type z = 0;

            for (Generation gj{0}; gj + 1 < config_.n_generation_; ++gj) {
                z += mlog_[role::to_space()][gj]->size();
            }

            return z;
        }

        void
        MutationLogState::visit_pools(const MemorySizeVisitor & visitor) const
        {
            for (uint32_t j = 0; j + 1 < config_.n_generation_; ++j) {
                for (uint32_t i = 0; i < c_n_role + 1; ++i) {
                    mlog_storage_[i][j].visit_pools(visitor);
                }
            }
        }

        void
        MutationLogState::verify_ok(GCObjectStore * gco_store,
                                    VerifyStats * p_verify_stats) noexcept
        {
            // 4. scan mutation logs
            for (Generation g(0); g + 1 < config_.n_generation_; ++g) {
                const DArena * space = gco_store->get_space(role::to_space(), g);
                const DArena * from = gco_store->get_space(role::from_space(), g);

                // mutation log for generation g records *incoming* pointers
                // from more senior generations; includes objects from *this*
                // generation that are older (track since source promotes before
                // destination)
                //
                for (const MutationLogEntry & mrecd : *(mlog_[role::to_space()][g])) {
                    // mutation log entries are only valid until the next assignment
                    // at the source location. Superseded entry may now point
                    // somewhere else. The snapshot member must however point
                    // to this generation, since that's preserved as long as the
                    // log entry survives.

                    void * orig_data = mrecd.snap().data();
                    void * curr_data = *mrecd.p_data();

                    if (orig_data == curr_data) {
                        // live mlog entry must point to to-space

                        if (space->contains_allocated(orig_data)) {
                            ++(p_verify_stats->n_mlog_vital_);
                        } else if (from->contains(curr_data)) {
                            // verify failure.
                            ++(p_verify_stats->n_mlog_from_);
                        } else {
                            // verify failure.
                            ++(p_verify_stats->n_mlog_wild_);
                        }
                    } else {
                        // requirements on superseded log entry:
                        // - snapshot refers to to-space
                        //
                        // no requirements on current data, entry is superseded anyway
                        //
                        ++(p_verify_stats->n_mlog_stale_);
                    }
                }
            }
        } /*verify_ok*/

        void
        MutationLogState::append_mutation(Generation dest_g,
                                          void * parent,
                                          void ** addr,
                                          obj<AGCObject> rhs)
        {
            // mlog keyed by generation in which pointer _destination_ resides:
            // collection that moves destination generation around needs to also
            // update pointers such as this one
            //
            MutationLog * mlog = this->mlog_[role::to_space()][dest_g];

            mlog->push_back(MutationLogEntry(parent, addr, rhs));
        }

        void
        MutationLogState::swap_roles(Generation upto) noexcept
        {
            scope log(XO_DEBUG(true), xtag("upto", upto));

            for (Generation g = Generation{0}; g < upto; ++g) {
                log && log("swap roles", xtag("g", g));

                std::swap(mlog_[role::to_space()][g], mlog_[role::from_space()][g]);
            }
        }

        void
        MutationLogState::forward_mutation_log(DX1Collector * gc,
                                               Generation upto)
        {
            /** non-zero if at least one object was rescued (from any generation)
             *  by mutation log scan
             **/
            std::size_t work = 0;

            do {
                // on 1st iteration, for all generations:
                // - to_mlog, triage_mlog are empty

                for (Generation child_gen{0};
                     child_gen + 2 < config_.n_generation_;
                     ++child_gen) {

                    MutationLog * from_mlog = this->mlog_[role::from_space()][child_gen];

                    if (!from_mlog->empty()) {
                        MutationLog * to_mlog = this->mlog_[role::to_space()][child_gen];
                        MutationLog * triage_mlog = this->mlog_[c_n_role][child_gen];

                        auto stats = this->_forward_mutation_log_phase(gc,
                                                                       upto,
                                                                       child_gen,
                                                                       from_mlog,
                                                                       to_mlog,
                                                                       triage_mlog);

                        from_mlog->clear();

                        // {from_mlog, triage_mlog} reverse roles

                        std::swap(this->mlog_[role::from_space()][child_gen],
                                  this->mlog_[c_n_role][child_gen]);

                        work += stats.n_rescue_;
                    }
                }
            } while (work > 0);

            // here: reached fixpoints, any remaining triaged mlogs can be discarded
            for (Generation child_gen{0}; child_gen + 2 < config_.n_generation_; ++child_gen) {
                MutationLog * triage_mlog = this->mlog_[c_n_role][child_gen];

                triage_mlog->clear();
            }
        }

        MutationLogStatistics
        MutationLogState::_forward_mutation_log_phase(DX1Collector * gc,
                                                      Generation upto,
                                                      Generation child_gen,
                                                      MutationLog * from_mlog,
                                                      MutationLog * keep_mlog,
                                                      MutationLog * triage_mlog)
        {
            scope log(XO_DEBUG(config_.debug_flag_),
                      xtag("child_gen", child_gen),
                      xtag("mlog.size", from_mlog->size()));

            /* categorize each mlog entry based on combination of {src, dest}.
             * In each case we care about {gen, age} of {src, dest}
             * objects.
             *
             * Enough cases to deserve a table:
             *
             * Legend:
             * - P        : parent object
             * - P'       : parent object after this gc phase
             * - g(P)     : generation of parent P.
             *              '+' if gen > child_gen (parent gen not collected this cycle)
             * - age(P)   : age of parent P.
             *
             * - C        : child object
             * - C'       : child object after this gc phase
             * - g(C)     : generation of child C.
             * - age(C)   : age of child C.
             *
             * - 0        : *from_mlog targets this object's generation.
             *              object not eligible for promotion.
             *              Write self* for objects eligible that promote
             *              if they survive this gc cycle.
             *              Write + for 'any generation senior to target'
             * - 1        : *from_mlog target this object's generation;
             *              object promotes if it survives
             *
             * - role     : 'to' this phase evacuated
             *                   (or in generation not eligible for collection)
             *              'fr' otherwise
             *
             *        | mlog | par   |              |      | mlog    | upd
             * case   | cur  | g(P)  | P      C     | C'   | action  | P      | move
             * -------+------+-------+--------------+------+---------+--------+-----
             * MLOG0  |  no  |       |              |      | discard |        | -
             *        |      |       |              |      |         |        |
             * MLOG1  | yes  | *     | to:+   fwd:* | to   | keep    | P->C'  | -
             * MLOG2  | yes  |       |        fr:0  | to   | keep    | P->C'  | C->to
             * MLOG3  | yes  | *     | fwd:*  -     | to   | update  | P'->C' | -
             * MLOG4  | yes  |       | fr:*   -     | -    | triage  | -      | -

             * notes:
             * MLOG1 : child C already forwarded (whether or not promoted)
             * MLOG2 : child C survives (and perhaps promoted).
             *         kept alive by parent in more-senior generation
             * MLOG3 : parent has been forwarded.
             *         update mlog entry for new parent location
             * MLOG4 : parent provisionally garbage. triage mlog entry until
             *         definite outcome.
             */

            MutationLogStatistics counters;
            // index of current mlog entry during evac
            std::uint32_t i_from = 0;

            for (MutationLogEntry & from_entry : *from_mlog) {
                if (log) {
                    log(xtag("i_from", i_from));
                }

                if (from_entry.is_superseded()) {
                    // there must be a second mlog entry that refers to
                    // the new child. Rely on that second entry,
                    // skipping this one.

                    // [MLOG0] obsolete mutation -> skip
                    ++counters.n_stale_;
                    continue;
                }

                /* here: mlog current */

                Generation parent_gen_to = gc->generation_of(role::to_space(),
                                                               from_entry.parent());

                if (parent_gen_to.is_sentinel()) {
                    void * parent_fr = *from_entry.p_data();

                    AllocInfo parent_info = gc->alloc_info((std::byte *)parent_fr);

                    if (parent_info.is_forwarding_tseq()) {
                        /* [MLOG3] */

                        ++counters.n_live_parent_;

                        // new parent location in to-space
                        // TODO: method on AllocInfo to streamline this
                        void * parent_to = *(void **)parent_fr;

                        parent_gen_to = gc->generation_of(role::to_space(),
                                                            parent_to);
                        parent_info = gc->alloc_info((std::byte *)parent_to);

                        assert(!parent_gen_to.sentinel());

                        // Since parent already forwarded, we don't have to preserve child
                        // or update parent object.
                        //
                        // Do need to replace mlog entry to reflect new parent location.

                        std::size_t offset
                            = ((std::byte *)from_entry.p_data()
                               - (std::byte *)from_entry.parent());

                        void ** p_data_to = (void **)((std::byte *)(parent_to) + offset);
                        void * child_to = *p_data_to;

                        MutationLogEntry to_entry(parent_to, p_data_to, from_entry.snap());

                        this->_check_keep_mutation_aux(gc->gco_store(),
                                                       to_entry,
                                                       parent_gen_to,
                                                       child_to,
                                                       keep_mlog);


                    } else {
                        ++counters.n_triage_;

                        // parent hasn't been collected and may be garbage.
                        // However this is only provisional, since
                        // parent could turn out to be reachable via some other mutation.

                        triage_mlog->push_back(from_entry);
                    }
                } else {
                    /* [MLOG1, MLOG2] */

                    counters += this->_preserve_child_of_live_parent(gc,
                                                                     upto,
                                                                     parent_gen_to,
                                                                     from_entry,
                                                                     keep_mlog);
                }
            }

            return counters;
        }

        MutationLogStatistics
        MutationLogState::_preserve_child_of_live_parent(DX1Collector * gc,
                                                         Generation upto,
                                                         Generation parent_gen,
                                                         const MutationLogEntry & from_entry,
                                                         MutationLog * keep_mlog)
        {
            void * child_fr = *from_entry.p_data();
            AllocInfo child_info = gc->alloc_info((std::byte *)(child_fr));

            MutationLogStatistics counters;

            // if child collected: new child location in to-space
            void * child_to = nullptr;

            // parent is alive: gc must ensure child remains alive

            ++counters.n_live_parent_;

            // Parent already recognized as alive. Either not subject to collection
            // or already evacuated.
            // (+ remember this need not be 1st pass over mlog entries)

            if (child_info.is_forwarding_tseq()) {
                // [MLOG1]

                // child already forwarded.
                // TODO: make this a method on AllocInfo
                child_to  = *(void **)child_fr;

                // assigning through address of P->C pointer
                // also makes mlog entry current

            } else {
                // [MLOG2]

                ++counters.n_rescue_;

                child_to = gc->deep_move_interior(child_fr, upto);

                // update child pointer in parent object
                *from_entry.p_data() = child_to;
            }

            // child_to generation in {gen, gen+1}

            this->_check_keep_mutation_aux(gc->gco_store(),
                                           from_entry, parent_gen, child_to, keep_mlog);

            return counters;
        }

        bool
        MutationLogState::_check_keep_mutation_aux(const GCObjectStore & gco_store,
                                                   const MutationLogEntry & from_entry,
                                                   Generation parent_gen_to,
                                                   void * child_to,
                                                   MutationLog * keep_mlog)
        {
            Generation child_gen_to
                = gco_store.generation_of(role::to_space(), child_to);

            bool need_mlog_entry
                = ((child_gen_to + 1 < config_.n_generation_)
                   && (config_.promotion_threshold(parent_gen_to)
                       > config_.promotion_threshold(child_gen_to)));

            if (need_mlog_entry) {
                // 1. P->C pointer is still cross-age (xage), and
                // 2. this matters; in future P will promote before C
                //
                // Need to keep entry because parent will be eligible for promotion
                // before child

                keep_mlog->push_back(from_entry);

                return true;
            } else {
                // child now in final generation,
                // no longer need to track incoming mutations.

                return false;
            }
        }


    } /*namespace mm*/
} /*namespace xo*/

/* end MutationLogState.cpp */
