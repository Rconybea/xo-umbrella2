/** @file MutationLogStore.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include "MutationLogStore.hpp"
#include "X1Collector.hpp" // temporary

namespace xo {
    namespace mm {

        MutationLogStore::MutationLogStore(const MutationLogConfig & config,
                                           GCObjectStore * gco_store)
        : config_{config},
          gco_store_{gco_store}
        {}

        void
        MutationLogStore::init_mlogs(std::size_t page_z)
        {
            assert(c_n_role + 1 == 3);

            for (uint32_t igen = 0, ngen = config_.n_generation_; igen + 1 < ngen; ++igen) {
                // special case: no use for mutation log for youngest generation,
                // so don't trouble to allocate one

                if (igen + 1 < c_max_generation) {
                    std::array<char, 3> label_v{'a', 'b', 'c'};

                    for (std::uint32_t mlog_role = 0; mlog_role < c_n_role + 1; ++mlog_role) {
                        this->mlog_storage_[mlog_role][igen]
                            = this->_make_mlog(igen,
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
        MutationLogStore::_make_mlog(uint32_t igen, char tag_char,
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
        MutationLogStore::mutation_log_entries() const noexcept -> size_type
        {
            size_type z = 0;

            for (Generation gj{0}; gj + 1 < config_.n_generation_; ++gj) {
                z += mlog_[Role::to_space()][gj]->size();
            }

            return z;
        }

        void
        MutationLogStore::visit_pools(const MemorySizeVisitor & visitor) const
        {
            for (uint32_t j = 0; j + 1 < config_.n_generation_; ++j) {
                for (uint32_t i = 0; i < c_n_role + 1; ++i) {
                    mlog_storage_[i][j].visit_pools(visitor);
                }
            }
        }

        void
        MutationLogStore::verify_ok() noexcept
        {
            X1VerifyStats * p_verify_stats = gco_store_->verify_stats();

            // 4. scan mutation logs
            for (Generation g(0); g + 1 < config_.n_generation_; ++g) {
                const DArena * space = gco_store_->get_space(Role::to_space(), g);
                const DArena * from = gco_store_->get_space(Role::from_space(), g);

                // mutation log for generation g records *incoming* pointers
                // from more senior generations; includes objects from *this*
                // generation that are older (track since source promotes before
                // destination)
                //
                for (const MutationLogEntry & mrecd : *(mlog_[Role::to_space()][g])) {
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
        MutationLogStore::assign_member_aux(GCObjectStore * gco_store,
                                            void * parent,
                                            AGCObject * lhs_iface,
                                            void ** lhs_addr,
                                            AGCObject * rhs_iface,
                                            void * rhs_data)
        {
            scope log(XO_DEBUG(config_.debug_flag_),
                      xtag("parent", parent),
                      xtag("lhs.iface", lhs_iface),
                      xtag("&lhs.data", lhs_addr),
                      xtag("rhs.iface", rhs_iface),
                      xtag("rhs.data", rhs_data));

            // ++ stats.n_mutation_;

            assert(parent);
            assert(lhs_addr);
            assert(rhs_iface);
            assert(rhs_data);

            if (lhs_iface) {
                // memcpy (not assignment): lhs_iface points to AGCObject storage
                // whose vptr was set at construction (e.g. IGCObject_Any from
                // a default-constructed obj<AGCObject>). Polymorphic copy-assignment
                // copies AGCObject's data members but NOT the vptr, so it would
                // leave the slot dispatching to the wrong (often fatal) iface.
                ::memcpy((void *)lhs_iface, (void *)rhs_iface, sizeof(AGCObject));
            }

            *lhs_addr = rhs_data;

            if (!config_.enabled_flag_) {
                log && log(xtag("msg", "noop b/c incremental gc disabled"));

                // only need to log mutations when incremental gc is enabled
                return;
            }

            // logging policy depends on:
            // 1. generation of lhs
            // 2. generation of rhs

            Generation src_g = gco_store->generation_of(Role::to_space(), lhs_addr);

            if (src_g.is_sentinel()) {
                log && log(xtag("msg", "noop because src not gc-owned"));

                // only need mlog entries for gc-owned pointers.
                // In this case pointer does not originate in gc-owned space
                return;
            }

            Generation dest_g = gco_store->generation_of(Role::to_space(), rhs_data);

            if (dest_g.is_sentinel()) {
                log && log(xtag("msg", "noop because dest not gc-owned"));

                // similarly, don't need mlog entry to non-gc-owned destination
                return;
            }

            if (dest_g + 1 == config_.n_generation_) {
                log && log(xtag("msg", "noop because dest in last gen"));

                // don't need mlog entry to final gen
                return;
            }

            if (src_g < dest_g) {
                log && log(xtag("msg", "noop because src gen younger than dest gen"));

                // young-to-old pointers don't need to be remembered,
                // since a GC cycle that collects an (old) generation is guarnatted
                // to also collect all younger generations.
                return;
            }

            if (src_g == dest_g) {
                // for pointers within the same generation, need to log
                // if source is older than destination.

                const DArena * arena = gco_store->get_space(Role::to_space(), src_g);

                const DArena::header_type * src_hdr = arena->obj2hdr(parent);
                const DArena::header_type * dest_hdr = arena->obj2hdr(rhs_data);

                assert(src_hdr && dest_hdr);

                if (gco_store->header2age(*src_hdr) <= gco_store->header2age(*dest_hdr)) {
                    log && log(xtag("msg", "noop because src age no older than dest age"));

                    // source and destination have the same age;
                    // therefore are always collected on the same set of GC cycles
                    // -> no need to remember separately.
                    return;
                } else {
                    // even though {src,dest} belong to the same generation:
                    // source will be eligible for promotion before destination.
                    // At that point pointer would become a cross-generational pointer,
                    // so need to track it now.

                    log && log("xage ptr -> must log");
                }
            } else {
                log && log("xgen ptr -> must log");
            }

            // control here: we have an older->younger pointer, need to log it

            obj<AGCObject> snap(rhs_iface, rhs_data);

            this->_append_mutation(dest_g, parent, lhs_addr, snap);
        }

        void
        MutationLogStore::_append_mutation(Generation dest_g,
                                           void * parent,
                                           void ** addr,
                                           obj<AGCObject> snap)
        {
            // mlog keyed by generation in which pointer _destination_ resides:
            // collection that moves destination generation around needs to also
            // update pointers such as this one
            //
            MutationLog * mlog = this->mlog_[Role::to_space()][dest_g];

            mlog->push_back(MutationLogEntry(parent, addr, snap));
        }

        void
        MutationLogStore::swap_roles(Generation upto) noexcept
        {
            scope log(XO_DEBUG(config_.debug_flag_), xtag("upto", upto));

            for (Generation g = Generation{0}; g < upto; ++g) {
                log && log("swap roles", xtag("g", g));

                std::swap(mlog_[Role::to_space()][g], mlog_[Role::from_space()][g]);
            }
        }

        void
        MutationLogStore::forward_mutation_log(obj<AGCObjectVisitor> gc,
                                               Generation upto)
        {
            scope log0(XO_DEBUG(config_.debug_flag_));

            /** non-zero if at least one object was rescued (from any generation)
             *  by mutation log scan
             **/
            std::size_t work = 0;

            /** count outer loop iterations */
            std::size_t i_fixpoint_loop = 0;

            do {
                scope log1(XO_DEBUG(log0), "fixpoint", xtag("i", i_fixpoint_loop));

                work = 0;

                // on 1st iteration, for all generations:
                // - to_mlog, triage_mlog are empty

                for (Generation child_gen{0};
                     child_gen + 1 < config_.n_generation_;
                     ++child_gen) {

                    scope log2(XO_DEBUG(log1), xtag("gen", child_gen));

                    MutationLog * from_mlog
                        = this->mlog_[Role::from_space()][child_gen];

                    if (!from_mlog->empty()) {
                        MutationLog * to_mlog = this->mlog_[Role::to_space()][child_gen];
                        MutationLog * triage_mlog = this->mlog_[c_n_role][child_gen];

                        auto stats = this->_forward_mutation_log_phase(gc,
                                                                       upto,
                                                                       child_gen,
                                                                       from_mlog,
                                                                       to_mlog,
                                                                       triage_mlog);

                        from_mlog->clear();

                        // {from_mlog, triage_mlog} reverse roles

                        std::swap(this->mlog_[Role::from_space()][child_gen],
                                  this->mlog_[c_n_role][child_gen]);

                        work += stats.n_rescue_;
                    }
                }

                ++i_fixpoint_loop;
            } while (work > 0);

            // here: reached fixpoints, any remaining triaged mlogs can be discarded
            for (Generation child_gen{0}; child_gen + 2 < config_.n_generation_; ++child_gen) {
                log0 && log0("dismiss unelected triage", xtag("gen", child_gen));

                MutationLog * triage_mlog = this->mlog_[c_n_role][child_gen];

                triage_mlog->clear();
            }
        }

        MutationLogStatistics
        MutationLogStore::_forward_mutation_log_phase(obj<AGCObjectVisitor> gc,
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
                    log(xtag("i_from", i_from),
                        xtag("parent", from_entry.parent()),
                        xtag("snap", from_entry.snap().data()));
                }

                // fixup gc-owned forwarded pointers in from_entry.
                // May update from_entry.p_data_, but not *(from_entry.p_data_), since:
                // 1. *p_data_ may not be gc-owned
                // 2. want to preserve ability to superseded mlog entry
                //
                // load-bearing at least for [MLOG3]
                //
                from_entry.check_forward_inplace(gco_store_, &counters);

                // Two possibiities for parent in to-space:
                // 1. belongs to generation not subject to collection this cycle.
                // 2. from_entry updated above for new location
                //
                Generation parent_gen_to = gc.generation_of(Role::to_space(),
                                                            from_entry.parent());

                if (parent_gen_to.is_sentinel()) {
                    // parent is not in to-space.
                    // Only gc-owned parents eligible for mlog entries.
                    // Therefore parent must be in from-space
                    // Since not rescued, it may be garbage.

                    log && log("parent not in to-space -> must be in from-space");

#                  ifndef NDEBUG
                    Generation parent_gen_from = gc.generation_of(Role::from_space(),
                                                                  from_entry.parent());
                    if (!parent_gen_from.is_sentinel())
                        assert(false);
#                  endif

                    if (from_entry.is_superseded()) {
                        log && log("entry superseded -> discard");

                        // parent mutated again after from_entry.
                        // If new child needs rescue, that will rely on mlog
                        // entry for that second mutation

                        ++counters.n_stale_;
                    } else {
                        log && log("entry current -> triage");

                        // although parent appears to be garbage,
                        // it may get rescued via some other mlog entry.
                        // Keep mlog entry while considering other mutations.

                        ++counters.n_triage_;

                        triage_mlog->push_back(from_entry);
                    }
                } else {
                    // parent in to-space: p_data_ is valid, can check superseded

                    log && log("parent in to-space");

                    if (from_entry.is_superseded()) {
                        log && log("entry superseded -> discard");
                        // there must be a second mlog entry that refers to
                        // the new child. Rely on that second entry,
                        // skipping this one.

                        // [MLOG0] obsolete mutation -> skip
                        ++counters.n_stale_;
                    } else {
                        log && log("entry current -> preserve child");

                        /* [MLOG1, MLOG2] */

                        log && log(xtag("case", "MLOG1|MLOG2"));

                        counters
                            += this->_preserve_child_of_live_parent(gc,
                                                                    upto,
                                                                    parent_gen_to,
                                                                    from_entry,
                                                                    keep_mlog);
                    }
                }
            }

            return counters;
        } /*forward_mutation_log_phase*/

        MutationLogStatistics
        MutationLogStore::_preserve_child_of_live_parent(obj<AGCObjectVisitor> gc,
                                                         Generation upto,
                                                         Generation parent_gen,
                                                         MutationLogEntry & from_entry,
                                                         MutationLog * keep_mlog)
        {
            scope log(XO_DEBUG(config_.debug_flag_));

            void * child_fr = *from_entry.p_data();
            AllocInfo child_info = gc.alloc_info((std::byte *)(child_fr));

            MutationLogStatistics counters;

            // if child collected: new child location in to-space
            void * child_to = nullptr;

            // parent is alive: gc must ensure child remains alive

            ++counters.n_live_parent_;

            // Parent already recognized as alive. Either not subject to collection
            // or already evacuated.
            // (+ remember this need not be 1st pass over mlog entries)

            //GCObjectStore & gco_store = x1gc->gco_store();

            if (child_info.is_forwarding_tseq()) {
                // [MLOG1]

                log && log(xtag("case", "MLOG1"), xtag("msg", "child forwarded"));

                // child already forwarded.
                // TODO: make this a method on AllocInfo
                child_to  = *(void **)child_fr;

            } else {
                // [MLOG2]

                log && log(xtag("case", "MLOG2"), xtag("msg", "rescue child"));

                ++counters.n_rescue_;

                child_to = gco_store_->deep_move_interior(gc, child_fr, upto);
            }

            // update child pointer in parent object.
            // either forwarded or moved
            *(from_entry.p_data()) = child_to;

            // TODO: pass statistics object
            if (from_entry.refresh_snapshot(parent_gen, gco_store_)) {
                keep_mlog->push_back(from_entry);
            }

            // child_to generation in {gen, gen+1}

            return counters;
        }

        void
        MutationLogStore::clear()
        {
            // parallels .init_mlogs(), see also

            for (uint32_t igen = 0, ngen = config_.n_generation_; igen + 1 < ngen; ++igen) {
                if (igen + 1 < c_max_generation) {
                    for (std::uint32_t mlog_role = 0; mlog_role < c_n_role + 1; ++mlog_role) {
                        this->mlog_storage_[mlog_role][igen].clear();
                    }
                }
            }
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end MutationLogStore.cpp */
