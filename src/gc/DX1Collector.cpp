/** @file DX1Collector.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "X1Collector.hpp"
#include <xo/gc/DX1CollectorIterator.hpp>

#include <xo/object2/Dictionary.hpp>
#include <xo/object2/Array.hpp>
#include <xo/object2/Integer.hpp>
#include <xo/object2/Boolean.hpp>
#include <xo/stringtable2/String.hpp>

#include <xo/alloc2/GCObject.hpp>
#include <xo/alloc2/Allocator.hpp>
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
          mlog_store_{cfg.mlog_config()},
          gco_store_{cfg.gco_store_config()}
        {
            assert(config_.arena_config_.header_.size_bits_ +
                   config_.arena_config_.header_.age_bits_ +
                   config_.arena_config_.header_.tseq_bits_ <= 64);

            size_t page_z = getpagesize();

            //this->_init_object_types(cfg, page_z);
            this->_init_gc_roots(cfg, page_z);
            this->_init_mlogs(page_z);
        }

#ifdef OBSOLETE // called from GCObjectStore ctor
        void
        DX1Collector::_init_object_types(const X1CollectorConfig & cfg, std::size_t page_z)
        {
            gco_state_._init_object_types();

#ifdef MOVED
            /* 1MB reserved address space enough for up to 128k distinct types.
             * In this case don't want to use hugepages since actual #of types
             * likely << .size/8
             */
            this->object_types_
                = ObjectTypeTable::map(ArenaConfig{.name_ = "x1-object-types",
                                                   .size_ = cfg.object_types_z_,
                                                   .hugepage_z_ = page_z,
                                                   .store_header_flag_ = false});
#endif
        }
#endif

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
        DX1Collector::contains(role r, const void * addr) const noexcept
        {
            return gco_store_.contains(r, addr);
        }

        bool
        DX1Collector::contains_allocated(role r, const void * addr) const noexcept
        {
            return gco_store_.contains_allocated(r, addr);
        }

        Generation
        DX1Collector::generation_of(role r, const void * addr) const noexcept
        {
            return gco_store_.generation_of(r, addr);
        }

        AllocError
        DX1Collector::last_error() const noexcept
        {
            // TODO:
            // need to adjust here if runtime errors
            // encountered during gc.

            return get_space(role::to_space(), Generation::nursery())->last_error_;
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

                for (role ri : role::all()) {
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
                        role r)
            {
                const DArena * arena = d.get_space(r, g);

                if (arena) [[likely]]
                    return (arena->*getter)();

                return 0;
            }
        }

        size_type
        DX1Collector::allocated(Generation g, role r) const noexcept
        {
            return stat_helper(*this, &DArena::allocated, g, r);
        }

        size_type
        DX1Collector::committed(Generation g, role r) const noexcept
        {
            return stat_helper(*this, &DArena::committed, g, r);
        }

        size_type
        DX1Collector::reserved(Generation g, role r) const noexcept
        {
            return stat_helper(*this, &DArena::reserved, g, r);
        }

        std::int32_t
        DX1Collector::locate_address(const void * addr) const noexcept
        {
            Generation g;

            g = this->generation_of(role::to_space(), addr);

            if (!g.is_sentinel())
                return g;

            g = this->generation_of(role::from_space(), addr);

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
                    for (role rj : role::all()) {
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

#ifdef MOVED
            scope log(XO_DEBUG(true));

            (void)error_mm;

            bool ok = true;

            // stats, indexed by tseq
            // could use c++ vector in scratch space instead of running on
            // boxed types.
            //
            DArray * stats_v = DArray::empty(mm, object_types_.size());

            if (!stats_v)
                return false;

            stats_v->resize(stats_v->capacity());

            log && log(xtag("object_types_.size", object_types_.size()),
                       xtag("stats_v.capacity", stats_v->capacity()),
                       xtag("stats_v.size", stats_v->size()));

            // count #of occupied type slots
            std::uint32_t n_tseq_present = 0;
            // largest tseq present with non-null AGCObject* iface
            std::int32_t max_tseq = 0;

            for (const ObjectTypeSlot & slot : object_types_) {
                AGCObject * iface = slot.iface();

                if (iface) {
                    typeseq tseq = iface->_typeseq();

                    ++n_tseq_present;
                    if (max_tseq < tseq.seqno())
                        max_tseq = tseq.seqno();

                    assert(tseq.seqno() >= 0);

                    auto tname_sv = TypeRegistry::id2name(tseq);
                    DString * tname = DString::from_view(mm, tname_sv);

                    DDictionary * recd = DDictionary::make(mm);

                    if (!recd)
                        return false;

                    recd->upsert_cstr(mm, "name", obj<AGCObject,DString>(tname));
                    recd->upsert_cstr(mm, "tseq", DInteger::box(mm, tseq.seqno()));
                    recd->upsert_cstr(mm, "n-live", DInteger::box(mm, 0));
                    recd->upsert_cstr(mm, "bytes", DInteger::box(mm, 0));

                    stats_v->assign_at(tseq.seqno(), obj<AGCObject,DDictionary>(recd));
                }
            }

            // scan to-space, count objects by type

            for (Generation g{0}; g < config_.n_generation_; ++g) {
                const DArena * arena = this->get_space(role::to_space(), g);

                for (AllocInfo info : *arena) {
                    if (info.is_forwarding_tseq()) {
                        assert(false);
                        return false;
                    }

                    uint32_t ix = info.tseq();
                    size_t z = info.size();

                    auto recd = obj<AGCObject,DDictionary>::from(stats_v->at(ix));

                    assert(recd);

                    auto n_live_opt = recd->lookup_cstr("n-live");
                    assert(n_live_opt);
                    auto bytes_opt = recd->lookup_cstr("bytes");
                    assert(bytes_opt);

                    if (n_live_opt && bytes_opt) {
                        auto n_live_gco = obj<AGCObject,DInteger>::from(n_live_opt.value());
                        auto bytes_gco = obj<AGCObject,DInteger>::from(bytes_opt.value());

                        n_live_gco->assign_value(n_live_gco->value() + 1);
                        bytes_gco->assign_value(bytes_gco->value() + z);
                    }
                }
            }

            stats_v->resize(max_tseq + 1);

            DArray * final_stats_v = DArray::empty(mm, n_tseq_present);

            for (std::size_t i = 0, n = stats_v->size(); i < n; ++i) {
                auto recd = stats_v->at(i);

                if (recd) {
                    bool ok = final_stats_v->push_back(recd);
                    assert(ok);
                }
            }

            *p_output = obj<AGCObject,DArray>(final_stats_v);

            return ok;
#endif
        }

        bool
        DX1Collector::report_object_ages(obj<AAllocator> mm,
                                         obj<AAllocator> error_mm,
                                         obj<AGCObject> * p_output) const noexcept
        {
            //return gco_store_.report_object_ages(mm, error_mm, p_output);

            scope log(XO_DEBUG(true));

            (void)error_mm;

            std::uint64_t n_age = config_.arena_config_.header_.max_age() + 1;

            // stats, indexed by age
            DArray * stats_v = DArray::empty(mm, n_age);

            if (!stats_v)
                return false;

            // pre-populate with empty dictionaries for each age bucket
            for (std::uint64_t a = 0; a < n_age; ++a) {
                DDictionary * recd = DDictionary::make(mm);

                if (!recd)
                    return false;

                recd->upsert_cstr(mm, "age", DInteger::box(mm, a));
                recd->upsert_cstr(mm, "n-live", DInteger::box(mm, 0));
                recd->upsert_cstr(mm, "bytes", DInteger::box(mm, 0));

                stats_v->push_back(obj<AGCObject,DDictionary>(recd));
            }

            log && log(xtag("n_age", n_age),
                       xtag("stats_v.size", stats_v->size()));

            // scan to-space, count objects by age

            // track largest age with at least one object
            std::int64_t max_age_present = 0;

            for (Generation g{0}; g < config_.n_generation_; ++g) {
                const DArena * arena = this->get_space(role::to_space(), g);

                for (AllocInfo info : *arena) {
                    if (info.is_forwarding_tseq()) {
                        assert(false);
                        return false;
                    }

                    uint32_t age = info.age();
                    size_t z = info.size();

                    if (static_cast<std::int64_t>(age) > max_age_present)
                        max_age_present = age;

                    auto recd = obj<AGCObject,DDictionary>::from(stats_v->at(age));

                    assert(recd);

                    auto n_live_opt = recd->lookup_cstr("n-live");
                    assert(n_live_opt);
                    auto bytes_opt = recd->lookup_cstr("bytes");
                    assert(bytes_opt);

                    if (n_live_opt && bytes_opt) {
                        auto n_live_gco = obj<AGCObject,DInteger>::from(n_live_opt.value());
                        auto bytes_gco = obj<AGCObject,DInteger>::from(bytes_opt.value());

                        n_live_gco->assign_value(n_live_gco->value() + 1);
                        bytes_gco->assign_value(bytes_gco->value() + z);
                    }
                }
            }

            // trim to only report ages up to max observed
            stats_v->resize(max_age_present + 1);

            *p_output = obj<AGCObject,DArray>(stats_v);

            return true;
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
        DX1Collector::alloc_info(value_type mem) const noexcept {
            return gco_store_.alloc_info(mem);
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

            auto self = this->ref<ACollector>();

            GCRunState saved_runstate = runstate_;
            {
                this->runstate_ = GCRunState::verify();
                this->verify_stats_.clear();

                // 2. visit roots
                for (GCRoot & root_slot : root_set_) {
                    VerifyStats pre = verify_stats_;

                    auto gco = *root_slot.root();

                    if (gco) {
                        // forward_children is hijacked here to verify
                        // pointer validity.
                        //
                        // Nested control re-enters
                        // - X1Collector::forward_inplace() -> _verify_aux()
                        //

                        gco.forward_children(self);

                    }

                    VerifyStats post = verify_stats_;

                    // assert fail -> root contains ptr to from-space
                    assert(pre.n_from_ == post.n_from_);

                    ++verify_stats_.n_gc_root_;
                }

                // 3. scan to-space for each generation
                for (Generation g(0); g < config_.n_generation_; ++g) {
                    const DArena * space = this->get_space(role::to_space(), g);

                    for (const AllocInfo & info : *space) {

                        if (info.is_forwarding_tseq()) {
                            ++verify_stats_.n_fwd_;

                        } else {
                            typeseq tseq(info.tseq());

                            const AGCObject * iface = this->lookup_type(tseq);

                            if (iface && !(iface->_has_null_vptr())) {
                                const void * data = info.payload().first;

                                // assembled fop for gc-aware object
                                obj<AGCObject> gco(iface, const_cast<void *>(data));

                                // forward_children is hijacked here to verify
                                // child pointer validity.
                                //
                                // Nested control reenters
                                // X1Collector::forward_inplace() -> _verify_aux()
                                //
                                gco.forward_children(self);
                            } else {
                                ++verify_stats_.n_no_iface_;
                                continue;
                            }
                        }
                    }
                }

                // 4. scan mutation logs
                mlog_store_.verify_ok(&gco_store_,
                                      &(this->verify_stats_));
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
            scope log(XO_DEBUG(true), xtag("upto", upto));

            assert(!runstate_.is_running());

            //auto t0 = std::chrono::steady_clock::now();

            log && log("memory");
            auto visitor = [&log](const MemorySizeInfo & info) {
                log && log(xtag("resource", info.resource_name_),
                           xtag("used", info.used_),
                           xtag("alloc", info.allocated_),
                           xtag("commit", info.committed_),
                           xtag("resv", info.reserved_),
                           xtag("lo", info.lo_),
                           xtag("hi", info.hi_));
            };
            this->visit_pools(visitor);

            if (config_.sanitize_flag_) {
                log && log("step 0a : verify");
                this->verify_ok();

            }

            log && log("step 0b : update run state");
            this->runstate_ = GCRunState::gc_upto(upto);

            log && log("step 0c : [STUB] snapshot alloc state");

            log && log("step 0d : [STUB] scan for object statistics");

            log && log("step 1  : swap from/to roles (now to-space is empty)");
            this->swap_roles(upto);

            log && log(xtag("from_0", get_space(role::from_space(), Generation{0})->lo_),
                       xtag("to_0", get_space(role::to_space(), Generation{0})->lo_));

            log && log("step 2a : copy roots");
            this->copy_roots(upto);

            log && log("step 2b : [STUB] copy pinned");

            log && log("step 3  : [STUB] forward mutation log");
            this->forward_mutation_log(upto);

            log && log("step 4a : [STUB] run destructors");
            log && log("step 4b : [STUB] keep reachable weak pointers");

            log && log("step 5  : cleanup");
            this->_cleanup_phase(upto);

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
        DX1Collector::swap_roles(Generation upto) noexcept
        {
            scope log(XO_DEBUG(true), xtag("upto", upto));

            gco_store_.swap_roles(upto);
            mlog_store_.swap_roles(upto);
        }

        void
        DX1Collector::forward_mutation_log(Generation upto)
        {
            mlog_store_.forward_mutation_log(this, upto);
        }

        void
        DX1Collector::_cleanup_phase(Generation upto)
        {
            scope log(XO_DEBUG(true), xtag("upto", upto));

            this->gco_store_.cleanup_phase(upto, config_.sanitize_flag_);
            this->runstate_ = GCRunState::idle();
        }

        void *
        DX1Collector::_deep_move_root(obj<AGCObject> from_src,
                                      Generation upto)
        {
            // NOTE:
            // Some roots are non-gc-owned nodes.
            // GC must still visit immediate children of these nodes
            // to move gc-owned children.
            // This implements virtual root node feature,
            // intended to mitigate mutation log churn.

            scope log(XO_DEBUG(config_.debug_flag_));

            if (!from_src)
                return nullptr;

            bool src_in_from_space = this->contains(role::from_space(), from_src.data());

            if (src_in_from_space) {
                return _deep_move_gc_owned(from_src.data(), upto);
            } else {
                // we aren't moving from_src, it's not gc-owned.
                // However weare moving all its gc-owned children

                auto self = this->ref<ACollector>();

                GCMoveCheckpoint gray_lo_v = this->_snap_move_checkpoint(upto);

                from_src.forward_children(self);

                // For each generation g:
                //   traverse objects newer than gray_lo_v[g], to make sure children
                //   are forwarded.  Fixpoint reached when gray_lo_v[g] doesn't change.
                //   Remember that forwarding may promote objects to older generation,
                //   so need multiple passes
                //
                this->_forward_children_until_fixpoint(upto, gray_lo_v);

                return from_src.data();
            }
        }

        void *
        DX1Collector::deep_move_interior(void * from_src,
                                         Generation upto)
        {
            scope log(XO_DEBUG(config_.debug_flag_));

            if (!from_src)
                return nullptr;

            bool src_in_from_space = this->contains(role::from_space(), from_src);

            if (!src_in_from_space)
                return from_src;

            return _deep_move_gc_owned(from_src, upto);
        }

        /*
         * rules:
         * - from_src must be in from-space
         * - object type stored in alloc header
         * - return value is new location in to-space
         *
         * - preserving i/face pointer
         * - replace destination with forwarding pointer
         *
         * EDITOR: gc -> self
         */
        void *
        DX1Collector::_deep_move_gc_owned(void * from_src,
                                          Generation upto)
        {
            return gco_store_._deep_move_gc_owned(this, from_src, upto);
        } /*_deep_move_gc_owned*/

        auto
        DX1Collector::_snap_move_checkpoint(Generation upto) -> GCMoveCheckpoint
        {
            return gco_store_.snap_move_checkpoint(upto);
        }

        void
        DX1Collector::_forward_children_until_fixpoint(Generation upto,
                                                       const GCMoveCheckpoint & gray_lo_v)
        {
            // problem -- need object type lookup

            gco_store_._forward_children_until_fixpoint(this, upto, gray_lo_v);
        }

        void
        DX1Collector::copy_roots(Generation upto) noexcept
        {
            scope log(XO_DEBUG(true));

            for (RootSet::size_type i = 0, n = root_set_.size(); i < n; ++i) {
                GCRoot & slot = root_set_[i];

                log && log("copy root",
                           xtag("slot.root()", slot.root()),
                           xtag("slot.root()->data_", slot.root()->data_));

                void * root_to = this->_deep_move_root(*slot.root(), upto);

                slot.root()->reset_opaque(root_to);

                log && log(xtag("slot.root()->data_", slot.root()->data_));
            }
        }

        void
        DX1Collector::forward_inplace(AGCObject * lhs_iface,
                                      void ** lhs_data)
        {
            Generation upto = runstate_.gc_upto();

            if (runstate_.is_running()) {
                // called during collection phase
                this->_forward_inplace_aux(lhs_iface, lhs_data, upto);
            } else if (runstate_.is_verify()) {
                // called during verify_ok
                this->_verify_aux(lhs_iface, *lhs_data);
            } else {
                // should be unreachable
                assert(false);
            }
        }

        void
        DX1Collector::_forward_inplace_aux(AGCObject * lhs_iface,
                                           void ** lhs_data,
                                           Generation upto)
        {
            // upto == runstate_.gc_upto()

            gco_store_._forward_inplace_aux(this, lhs_iface, lhs_data, upto);
        } /*_forward_inplace_aux*/

        void
        DX1Collector::_verify_aux(AGCObject * iface, void * data)
        {
            //scope log(XO_DEBUG(config_.debug_flag_), xtag("data", data));

            (void)iface;
            (void)data;

            Generation g1 = this->generation_of(role::to_space(), data);

            if (g1.is_sentinel()) {
                assert(this->contains(role::to_space(), data) == false);

                Generation g2 = this->generation_of(role::from_space(), data);

                if (!g2.is_sentinel()) {
                    // verify failure - live pointer still refers to from-space

                    ++(verify_stats_.n_from_);
                } else {
                    ++(verify_stats_.n_ext_);
                }
            } else {
                assert(this->contains(role::to_space(), data));

                ++(verify_stats_.n_to_);
            }
        }

        void *
        DX1Collector::_shallow_move(const AGCObject * iface, void * from_src)
        {
            return gco_store_._shallow_move(this, iface, from_src);
        }

        bool
        DX1Collector::check_move_policy(header_type alloc_hdr,
                                        void * object_data) const noexcept
        {
            assert(runstate_.is_running());

            return gco_store_._check_move_policy(alloc_hdr,
                                                 object_data,
                                                 runstate_.gc_upto());
        }

        auto
        DX1Collector::alloc(typeseq t, size_type z) noexcept -> value_type
        {
            return with_facet<AAllocator>::mkobj(new_space()).alloc(t, z);
        }

        auto
        DX1Collector::super_alloc(typeseq t, size_type z) noexcept -> value_type {
            return with_facet<AAllocator>::mkobj(new_space()).super_alloc(t, z);
        }

        auto
        DX1Collector::sub_alloc(size_type z, bool complete) noexcept -> value_type {
            return with_facet<AAllocator>::mkobj(new_space()).sub_alloc(z, complete);
        }

        auto
        DX1Collector::alloc_copy(value_type src) noexcept -> value_type {
            return with_facet<AAllocator>::mkobj(new_space()).alloc_copy(src);
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
            scope log(XO_DEBUG(config_.debug_flag_),
                      xtag("parent", parent), xtag("lhs", p_lhs), xtag("rhs", rhs.data()));

            // ++ stats.n_mutation_;

            *p_lhs = rhs;

            if (runstate_.is_running()) {
                // for removal of all doubt:
                // don't log mutations during GC cycle
                return;
            }

            if (!config_.allow_incremental_gc_) {
                // only need to log mutations when incremental gc is enabled
                return;
            }

            // logging policy depends on:
            // 1. generation of lhs
            // 2. generation of rhs

            Generation src_g = this->generation_of(role::to_space(), p_lhs);

            if (src_g.is_sentinel()) {
                // only need mlog entries for gc-owned pointers.
                // In this case pointer does not originate in gc-owned space
                return;
            }

            Generation dest_g = this->generation_of(role::to_space(), rhs.data());

            if (dest_g.is_sentinel()) {
                // similarly, don't need mlog entry to non-gc-owned destination
                return;
            }

            if (src_g < dest_g) {
                // young-to-old pointers don't need to be remembered,
                // since a GC cycle that collects an (old) generation is guarnatted
                // to also collect all younger generations.
                return;
            }

            if (src_g == dest_g) {
                // for pointers within the same generation, need to log
                // if source is older than destination.

                const DArena * arena = this->get_space(role::to_space(), src_g);

                const DArena::header_type * src_hdr = arena->obj2hdr(parent);
                const DArena::header_type * dest_hdr = arena->obj2hdr(rhs.data());

                assert(src_hdr && dest_hdr);

                if (this->header2age(*src_hdr) <= this->header2age(*dest_hdr)) {
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

            void ** lhs_addr = reinterpret_cast<void **>(&(p_lhs->data_));

            mlog_store_.append_mutation(dest_g, parent, lhs_addr, rhs);
        } /*assign_member*/

        DX1CollectorIterator
        DX1Collector::begin() const noexcept
        {
            scope log(XO_DEBUG(false));

            const DArena * arena
                = get_space(role::to_space(),
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
                = get_space(role::to_space(),
                            Generation(config_.n_generation_ - 1));
            DArenaIterator arena_end = arena->end();

            return DX1CollectorIterator(this,
                                        gen_hi,
                                        gen_hi,
                                        arena_end,
                                        arena_end);
        }

#ifdef MOVED
        void
        DX1Collector::reverse_roles(Generation g) noexcept {
            assert(g < config_.n_generation_);

            std::swap(space_[role::from_space()][g], space_[role::to_space()][g]);
        }
#endif

        void
        DX1Collector::clear() noexcept {
            for (role ri : role::all()) {
                for (Generation gj{0}; gj < config_.n_generation_; ++gj) {
                    DArena * arena = this->get_space(ri, gj);

                    assert(arena);

                    arena->clear();
                }
            }
        }
    } /*namespace mm*/
} /*namespace xo*/

/* end DX1Collector.cpp */
