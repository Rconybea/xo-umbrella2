/** @file DX1Collector.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "X1Collector.hpp"
#include "GCObjectStoreVisitor.hpp"
#include <xo/gc/DX1CollectorIterator.hpp>

#include <xo/object2/Dictionary.hpp>
#include <xo/object2/Array.hpp>
#include <xo/object2/Integer.hpp>
#include <xo/object2/Boolean.hpp>
#include <xo/stringtable2/String.hpp>

#include <xo/alloc2/GCObject.hpp>
//#include <xo/alloc2/Allocator_extra.hpp>
#include <xo/alloc2/Arena.hpp>
#include "object_age.hpp"
#include <xo/facet/obj.hpp>
#include <xo/indentlog/scope.hpp>
#include <cassert>
#include <cstdint>
#include <sys/mman.h>
#include <unistd.h> // for ::getpagesize()

namespace xo {
    // for report_statistics(), report_object_types()
    using xo::scm::DDictionary;
    using xo::scm::DArray;
    using xo::scm::DString;
    using xo::scm::DInteger;
    using xo::scm::DBoolean;

    using xo::mm::AAllocator;
    using xo::facet::TypeRegistry;
    using xo::facet::typeseq;
    using xo::facet::with_facet;

    namespace mm {

        // ----- GCRunState -----

        GCRunState::GCRunState(Mode mode, Generation gc_upto)
            : mode_{mode}, gc_upto_{gc_upto}
        {}

        GCRunState
        GCRunState::idle()
        {
            return GCRunState(Mode::idle, Generation::sentinel());
        }

        GCRunState
        GCRunState::verify()
        {
            return GCRunState(Mode::verify, Generation::sentinel());
        }

        GCRunState
        GCRunState::gc_upto(Generation g)
        {
            return GCRunState(Mode::gc, Generation(g + 1));
        }

        // ----- DX1Collector -----

        using size_type = xo::mm::DX1Collector::size_type;

        DX1Collector::DX1Collector(const X1CollectorConfig & cfg)
        : config_{cfg},
          gco_store_{cfg.gco_store_config(), &verify_stats_},
          mlog_store_{cfg.mlog_config(), &gco_store_}
        {
            assert(config_.arena_config_.header_.size_bits_ +
                   config_.arena_config_.header_.age_bits_ +
                   config_.arena_config_.header_.tseq_bits_ <= 64);

            size_t page_z = getpagesize();

            //this->_init_object_types(cfg, page_z);
            this->_init_gc_roots(cfg, page_z);
            this->_init_mlogs(page_z);
        }

        void
        DX1Collector::_init_gc_roots(const X1CollectorConfig & cfg, std::size_t page_z)
        {
            this->root_set_
                = RootSet::map(ArenaConfig{.name_ = "x1-object-roots",
                                          .size_ = cfg.object_roots_z_,
                                          .hugepage_z_ = page_z,
                                          .store_header_flag_ = false});
        }

        void
        DX1Collector::_init_mlogs(std::size_t page_z)
        {
            this->mlog_store_.init_mlogs(page_z);
        }

        void
        DX1Collector::visit_pools(const MemorySizeVisitor & visitor) const
        {
            //object_types_.visit_pools(visitor);
            root_set_.visit_pools(visitor);

            gco_store_.visit_pools(visitor);
            mlog_store_.visit_pools(visitor);
        }

        bool
        DX1Collector::contains(Role r, const void * addr) const noexcept
        {
            return gco_store_.contains(r, addr);
        }

        bool
        DX1Collector::contains_allocated(Role r, const void * addr) const noexcept
        {
            return gco_store_.contains_allocated(r, addr);
        }

        Generation
        DX1Collector::generation_of(Role r, const void * addr) const noexcept
        {
            return gco_store_.generation_of(r, addr);
        }

        AllocError
        DX1Collector::last_error() const noexcept
        {
            // TODO:
            // need to adjust here if runtime errors
            // encountered during gc.

            return get_space(Role::to_space(), Generation::nursery())->last_error_;
        }

        namespace {
            size_type
            accumulate_total_aux(const DX1Collector & d,
                                 size_t (DArena::* get_stat_fn)() const) noexcept
            {
                //size_t z1 = (d.object_types_.store()->*get_stat_fn)();
                size_t z1 = (d.gco_store_.get_object_types()->store()->*get_stat_fn)();
                size_t z2 = (d.root_set_.store()->*get_stat_fn)();

                size_t z3 = 0;

                for (Role ri : Role::all()) {
                    for (Generation gj{0}; gj < d.config_.n_generation_; ++gj) {
                        const DArena * arena = d.get_space(ri, gj);

                        assert(arena);

                        z3 += (arena->*get_stat_fn)();
                    }
                }

                return z1 + z2 + z3;
            }
        }

        size_type
        DX1Collector::reserved() const noexcept
        {
            return accumulate_total_aux(*this, &DArena::reserved);
        }

        size_type
        DX1Collector::size_total() const noexcept
        {
            return this->committed();
        }

        size_type
        DX1Collector::committed() const noexcept
        {
            return accumulate_total_aux(*this, &DArena::committed);
        }

        size_type
        DX1Collector::available() const noexcept
        {
            return accumulate_total_aux(*this, &DArena::available);
        }

        size_type
        DX1Collector::allocated() const noexcept
        {
            return accumulate_total_aux(*this, &DArena::allocated);
        }

        size_type
        DX1Collector::mutation_log_entries() const noexcept
        {
            return mlog_store_.mutation_log_entries();
        }

        namespace {
            size_type
            stat_helper(const DX1Collector & d,
                        size_type (DArena::* getter)() const,
                        Generation g,
                        Role r)
            {
                const DArena * arena = d.get_space(r, g);

                if (arena) [[likely]]
                    return (arena->*getter)();

                return 0;
            }
        }

        size_type
        DX1Collector::allocated(Generation g, Role r) const noexcept
        {
            return stat_helper(*this, &DArena::allocated, g, r);
        }

        size_type
        DX1Collector::committed(Generation g, Role r) const noexcept
        {
            return stat_helper(*this, &DArena::committed, g, r);
        }

        size_type
        DX1Collector::reserved(Generation g, Role r) const noexcept
        {
            return stat_helper(*this, &DArena::reserved, g, r);
        }

        std::int32_t
        DX1Collector::locate_address(const void * addr) const noexcept
        {
            Generation g;

            g = this->generation_of(Role::to_space(), addr);

            if (!g.is_sentinel())
                return g;

            g = this->generation_of(Role::from_space(), addr);

            if (!g.is_sentinel()) {
                // use negative values for

                return -1 - g;
            }

            return -1;
        }

        // editor bait: report-gc-statistics
        bool
        DX1Collector::report_statistics(obj<AAllocator> mm,
                                        obj<AAllocator> error_mm,
                                        obj<AGCObject> * p_output) const noexcept
        {
            (void)error_mm;

            DDictionary * rpt = DDictionary::make(mm);

            if (!rpt)
                return false;

            bool ok = true;

            // note: totals taken across both roles and generations,
            //       so counts both from-space and to-space
            //
            ok &= rpt->upsert_cstr(mm, "n-generation", DInteger::box(mm, config_.n_generation_));
            ok &= rpt->upsert_cstr(mm, "n-survive-threshold", DInteger::box(mm, config_.n_survive_threshold_));
            ok &= rpt->upsert_cstr(mm, "allow-incremental-gc", DBoolean::box(mm, config_.allow_incremental_gc_));
            ok &= rpt->upsert_cstr(mm, "sanitize", DBoolean::box(mm, config_.sanitize_flag_));
            ok &= rpt->upsert_cstr(mm, "allocated", DInteger::box(mm, this->allocated()));
            ok &= rpt->upsert_cstr(mm, "committed", DInteger::box(mm, this->committed()));
            ok &= rpt->upsert_cstr(mm, "reserved", DInteger::box(mm, this->reserved()));
            ok &= rpt->upsert_cstr(mm, "n-mlog-entry", DInteger::box(mm, this->mutation_log_entries()));

            // per-(generation,role) info
            {
                for (Generation gi{0}; gi < config_.n_generation_; ++gi) {
                    for (Role rj : Role::all()) {
                        const DArena * arena = this->get_space(rj, gi);
                        DDictionary * arena_d = DDictionary::make(mm);

                        auto lo = reinterpret_cast<DInteger::value_type>(arena->lo_);
                        auto free = reinterpret_cast<DInteger::value_type>(arena->free_);
                        auto limit = reinterpret_cast<DInteger::value_type>(arena->limit_);
                        auto hi = reinterpret_cast<DInteger::value_type>(arena->hi_);

                        ok &= arena_d->upsert_cstr(mm, "lo", DInteger::box(mm, lo));
                        ok &= arena_d->upsert_cstr(mm, "d-free", DInteger::box(mm, free - lo));
                        ok &= arena_d->upsert_cstr(mm, "d-limit", DInteger::box(mm, limit - lo));
                        ok &= arena_d->upsert_cstr(mm, "d-hi", DInteger::box(mm, hi - lo));

                        const DString * key = DString::from_str(mm, arena->config_.name_);

                        rpt->upsert(mm, std::make_pair(key, obj<AGCObject,DDictionary>(arena_d)));
                    }
                }
            }

            *p_output = obj<AGCObject,DDictionary>(rpt);

            return ok;
        }

        bool
        DX1Collector::report_object_types(obj<AAllocator> mm,
                                          obj<AAllocator> error_mm,
                                          obj<AGCObject> * p_output) const noexcept
        {
            return gco_store_.report_object_types(mm, error_mm, p_output);
        }

        bool
        DX1Collector::report_object_ages(obj<AAllocator> mm,
                                         obj<AAllocator> error_mm,
                                         obj<AGCObject> * p_output) const noexcept
        {
            return gco_store_.report_object_ages(mm, error_mm, p_output);
        }

        size_type
        DX1Collector::header2size(header_type hdr) const noexcept
        {
            return gco_store_.header2size(hdr);
        }

        object_age
        DX1Collector::header2age(header_type hdr) const noexcept
        {
            return gco_store_.header2age(hdr);
        }

        uint32_t
        DX1Collector::header2tseq(header_type hdr) const noexcept
        {
            return gco_store_.header2tseq(hdr);
        }

        bool
        DX1Collector::is_forwarding_header(header_type hdr) const noexcept
        {
            return gco_store_.is_forwarding_header(hdr);
        }

        AllocInfo
        DX1Collector::alloc_info(void * mem) const noexcept {
            return gco_store_.alloc_info((std::byte *)(mem));
        }

        bool
        DX1Collector::is_type_installed(typeseq tseq) const noexcept
        {
            return gco_store_.is_type_installed(tseq);
        }

        bool
        DX1Collector::verify_ok() noexcept
        {
            // 1. visit space pointers
            //    - verify space_[*] points to space_storage_[*]
            //    - verify mlog_[*] points to mlog_storage_[*]
            //
            // 2. visit roots:
            //    for each root, verify that immediate child pointers are in to-space
            //
            // 3. scan to-space:
            //    for each object, verify that immediate children are also in to-space
            //
            // 4. scan mutation logs:
            //    verify that entries refer to to-space

            // Each AGCObject impl provides a forward_children() method,
            // that calls DX1Collector::forward_inplace(iface, &data)
            //
            // tactical plan: hijack forward_children.
            // Add run state so DX1Collector can recognize forward_inplace()
            // calls made for the purpose of checking child pointers.

            DGCObjectStoreVisitor visitor(&gco_store_,
                                          Generation{0} /*not used for verify*/);

            GCRunState saved_runstate = runstate_;
            {
                this->runstate_ = GCRunState::verify();
                this->verify_stats_.clear();

                // 2. visit roots
                for (GCRoot & root_slot : root_set_) {
                    X1VerifyStats pre = verify_stats_;

                    auto gco = *root_slot.root();

                    if (gco) {
                        // forward_children is hijacked here to verify
                        // pointer validity.
                        //
                        // Nested control re-enters
                        // - X1Collector::forward_inplace() -> _verify_aux()
                        //

                        gco.visit_gco_children(VisitReason::verify(), visitor.ref());
                    }

                    X1VerifyStats post = verify_stats_;

                    // assert fail -> root contains ptr to from-space
                    assert(pre.n_from_ == post.n_from_);

                    ++verify_stats_.n_gc_root_;
                }

                // 3. scan to-space for each generation
                gco_store_.verify_ok();

                // 4. scan mutation logs
                mlog_store_.verify_ok();
            }

            // restore run state at end of verify cycle
            this->runstate_ = saved_runstate;

            bool ok = verify_stats_.is_ok();

            return ok;
        }

        const AGCObject *
        DX1Collector::lookup_type(typeseq tseq) const noexcept
        {
            return gco_store_.lookup_type(tseq);
        }

        /* editor bait: register_type */
        bool
        DX1Collector::install_type(const AGCObject & meta) noexcept
        {
            return gco_store_.install_type(meta);
        }

        void
        DX1Collector::add_gc_root_poly(obj<AGCObject> * p_root) noexcept
        {
            root_set_.push_back(GCRoot(p_root));
        }

        void
        DX1Collector::remove_gc_root_poly(obj<AGCObject> * p_root) noexcept
        {
            // iterate over roots_, find p_root and drop it

            (void)p_root;
        }

        void
        DX1Collector::request_gc(Generation upto) noexcept
        {
            if (gc_blocked_ > 0) {
                if (gc_pending_upto_ < upto) {
                    this->gc_pending_upto_ = upto;
                }

                /* intend collecting later */
            } else {
                this->execute_gc(upto);
            }
        }

        void
        DX1Collector::execute_gc(Generation upto) noexcept
        {
            scope log(XO_DEBUG(config_.debug_flag_), xtag("upto", upto));

            assert(!runstate_.is_running());

            //auto t0 = std::chrono::steady_clock::now();

            log && log("memory");
            auto resource_visitor = [&log](const MemorySizeInfo & info) {
                log && log(xtag("resource", info.resource_name_),
                           xtag("used", info.used_),
                           xtag("alloc", info.allocated_),
                           xtag("commit", info.committed_),
                           xtag("resv", info.reserved_),
                           xtag("lo", info.lo_),
                           xtag("hi", info.hi_));
            };
            this->visit_pools(resource_visitor);

            if (config_.sanitize_flag_) {
                log && log("step 0a : verify");
                this->verify_ok();

            }

            DGCObjectStoreVisitor gco_visitor(&gco_store_, upto);

            log && log("step 0b : update run state");
            this->runstate_ = GCRunState::gc_upto(upto);

            log && log("step 0c : [STUB] snapshot alloc state");

            log && log("step 0d : [STUB] scan for object statistics");

            log && log("step 1  : swap from/to roles (now to-space is empty)");
            this->_swap_roles(upto);

            log && log(xtag("from_0", get_space(Role::from_space(), Generation{0})->lo_),
                       xtag("to_0", get_space(Role::to_space(), Generation{0})->lo_));

            log && log("step 2a : copy roots");
            this->_copy_roots(upto);

            log && log("step 2b : [STUB] copy pinned");

            log && log("step 3  : [STUB] forward mutation log");
            mlog_store_.forward_mutation_log(gco_visitor.ref(), upto);

            log && log("step 4a : [STUB] run destructors");
            log && log("step 4b : [STUB] keep reachable weak pointers");

            log && log("step 5  : cleanup");
            this->_cleanup_phase(upto);

            log && log("step 6  : update gc statistics");
            gc_stats_.include_gc();

            if (config_.sanitize_flag_) {
                log && log("step 5b : verify");
                bool ok = this->verify_ok();

                log && log(xtag("n-gc-root", verify_stats_.n_gc_root_),
                           xtag("n-ext", verify_stats_.n_ext_),
                           xtag("n-from", verify_stats_.n_from_),
                           xtag("n-to", verify_stats_.n_to_),
                           xtag("n-fwd", verify_stats_.n_fwd_),
                           xtag("n-no-iface", verify_stats_.n_no_iface_),
                           xtag("n-mlog-vital", verify_stats_.n_mlog_vital_),
                           xtag("n-mlog-stale", verify_stats_.n_mlog_stale_),
                           xtag("n-mlog-from", verify_stats_.n_mlog_from_),
                           xtag("n-mlog-wild", verify_stats_.n_mlog_wild_));

                assert(ok);
            }
        }

        void
        DX1Collector::_swap_roles(Generation upto) noexcept
        {
            scope log(XO_DEBUG(config_.debug_flag_), xtag("upto", upto));

            gco_store_.swap_roles(upto);
            mlog_store_.swap_roles(upto);
        }

        void
        DX1Collector::_cleanup_phase(Generation upto)
        {
            scope log(XO_DEBUG(config_.debug_flag_), xtag("upto", upto));

            this->gco_store_.cleanup_phase(upto, config_.sanitize_flag_);
            this->runstate_ = GCRunState::idle();
        }

        void
        DX1Collector::_copy_roots(Generation upto) noexcept
        {
            scope log(XO_DEBUG(config_.debug_flag_));

            for (RootSet::size_type i = 0, n = root_set_.size(); i < n; ++i) {
                GCRoot & slot = root_set_[i];

                log && log("copy root",
                           xtag("slot.root()", slot.root()),
                           xtag("slot.root()->data_", slot.root()->data_));

                void * root_to = gco_store_.deep_move_root(slot.root()->iface(),
                                                           (void **)&(slot.root()->data_), upto);

                slot.root()->reset_opaque(root_to);

                log && log(xtag("slot.root()->data_", slot.root()->data_));
            }
        }

        auto
        DX1Collector::alloc(typeseq t, size_type z) noexcept -> value_type
        {
            return with_facet<AAllocator>::mkobj(new_space()).alloc(t, z);
        }

        auto
        DX1Collector::super_alloc(typeseq t, size_type z) noexcept -> value_type {
            return with_facet<AAllocator>::mkobj(this->new_space()).super_alloc(t, z);
        }

        auto
        DX1Collector::sub_alloc(size_type z, bool complete) noexcept -> value_type {
            return with_facet<AAllocator>::mkobj(this->new_space()).sub_alloc(z, complete);
        }

        auto
        DX1Collector::alloc_copy(value_type src) noexcept -> value_type {
            return this->new_space()->alloc_copy(src);
            //return with_facet<AAllocator>::mkobj(this->new_space()).alloc_copy(src);
        }

        bool
        DX1Collector::expand(size_type z) noexcept
        {
            if (with_facet<AAllocator>::mkobj(to_space(Generation{0})).expand(z))
                return with_facet<AAllocator>::mkobj(from_space(Generation{0})).expand(z);

            return false;
        }

        // editor bait: write barrier
        void
        DX1Collector::assign_member(void * parent, obj<AGCObject> * p_lhs, obj<AGCObject> rhs)
        {
            this->barrier_assign_aux(parent,
                                     p_lhs->iface(),
                                     p_lhs->opaque_data_addr(),
                                     rhs.iface(),
                                     rhs.opaque_data());
        } /*assign_member*/

        DX1CollectorIterator
        DX1Collector::begin() const noexcept
        {
            scope log(XO_DEBUG(false));

            const DArena * arena
                = get_space(Role::to_space(),
                            Generation{0});

            return DX1CollectorIterator(this,
                                        Generation{0},
                                        Generation{config_.n_generation_},
                                        arena->begin(),
                                        arena->end());
        }

        DX1CollectorIterator
        DX1Collector::end() const noexcept {
            scope log(XO_DEBUG(false));

            Generation gen_hi = Generation{config_.n_generation_};

            /** valid iterator for end points to end of last DArena.
             *  otherwise will interfere with working compare
             *  (since invalid iterators are incomparable)
             **/

            const DArena * arena
                = get_space(Role::to_space(),
                            Generation(config_.n_generation_ - 1));
            DArenaIterator arena_end = arena->end();

            return DX1CollectorIterator(this,
                                        gen_hi,
                                        gen_hi,
                                        arena_end,
                                        arena_end);
        }

        void
        DX1Collector::clear() noexcept {
            mlog_store_.clear();
            gco_store_.clear();
            root_set_.clear();
        }

        void
        DX1Collector::barrier_assign_aux(void * parent,
                                         AGCObject * lhs_iface, void ** lhs_data,
                                         AGCObject * rhs_iface, void * rhs_data)
        {
            scope log(XO_DEBUG(config_.debug_flag_),
                      xtag("parent", parent),
                      xtag("lhs.iface", lhs_iface), xtag("&lhs.data", lhs_data),
                      xtag("rhs.iface", rhs_iface), xtag("rhs.data", rhs_data));

            mlog_store_.assign_member_aux(&gco_store_,
                                          parent,
                                          lhs_iface,
                                          lhs_data,
                                          rhs_iface,
                                          rhs_data);
        }
    } /*namespace mm*/
} /*namespace xo*/

/* end DX1Collector.cpp */
