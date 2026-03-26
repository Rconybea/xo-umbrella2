/** @file DX1Collector.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include <xo/alloc2/GCObject.hpp>
#include <xo/alloc2/Allocator.hpp>
#include "detail/IAllocator_DX1Collector.hpp"
#include "detail/ICollector_DX1Collector.hpp"
#include "arena/IAllocator_DArena.hpp"
#include <xo/gc/DX1Collector.hpp>
#include <xo/gc/DX1CollectorIterator.hpp>
#include <xo/alloc2/generation.hpp>
#include "object_age.hpp"
#include <xo/facet/obj.hpp>
#include <xo/indentlog/scope.hpp>
#include <cassert>
#include <cstdint>
#include <sys/mman.h>
#include <unistd.h> // for ::getpagesize()

namespace xo {
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

        DX1Collector::DX1Collector(const X1CollectorConfig & cfg) : config_{cfg}
        {
            assert(config_.arena_config_.header_.size_bits_ +
                   config_.arena_config_.header_.age_bits_ +
                   config_.arena_config_.header_.tseq_bits_ <= 64);

            size_t page_z = getpagesize();

            this->_init_object_types(cfg, page_z);
            this->_init_gc_roots(cfg, page_z);
            this->_init_mlogs(cfg, page_z);
            this->_init_space(cfg);
        }

        void
        DX1Collector::_init_object_types(const X1CollectorConfig & cfg, std::size_t page_z)
        {
            /* 1MB reserved address space enough for up to 128k distinct types.
             * In this case don't want to use hugepages since actual #of types
             * likely << .size/8
             */
            this->object_types_
                = DArena::map(ArenaConfig{.name_ = "x1-object-types",
                                          .size_ = cfg.object_types_z_,
                                          .hugepage_z_ = page_z,
                                          .store_header_flag_ = false});
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
        DX1Collector::_init_mlogs(const X1CollectorConfig & cfg, std::size_t page_z)
        {
            for (uint32_t igen = 0, ngen = cfg.n_generation_; igen + 1 < ngen; ++igen) {
                // special case: no use for mutation log for youngest generation,
                // so don't trouble to allocate one

                if (igen + 1 < c_max_generation) {
                    this->mlog_storage_[0][igen] = _make_mlog(igen, 'a', cfg.mutation_log_z_, page_z);
                    this->mlog_storage_[1][igen] = _make_mlog(igen, 'b', cfg.mutation_log_z_, page_z);
                    this->mlog_storage_[2][igen] = _make_mlog(igen, 'c', cfg.mutation_log_z_, page_z);

                    this->mlog_[0][igen] = &mlog_storage_[0][igen];
                    this->mlog_[1][igen] = &mlog_storage_[1][igen];
                    this->mlog_[2][igen] = &mlog_storage_[2][igen];
                } else {
                    assert(false);
                }
            }

            if (cfg.n_generation_ > 0) {
                for (uint32_t igen = cfg.n_generation_ - 1; igen + 1 < c_max_generation; ++igen) {
                    this->mlog_[0][igen] = nullptr;
                    this->mlog_[1][igen] = nullptr;
                    this->mlog_[2][igen] = nullptr;
                }
            } else {
                assert(false);
            }
        }

        auto
        DX1Collector::_make_mlog(uint32_t igen, char tag_char, size_t mlog_z, size_t page_z) -> MutationLog
        {
            char buf[40];
            snprintf(buf, sizeof(buf), "x1-mlog-G%u-%c", igen, tag_char);

            return MutationLog::map(ArenaConfig{.name_ = std::string(buf),
                                                .size_ = mlog_z,
                                                .hugepage_z_ = page_z,
                                                .store_header_flag_ = false});
        }

        void
        DX1Collector::_init_space(const X1CollectorConfig & cfg)
        {
            assert(c_n_role == 2);

            for (uint32_t igen = 0, ngen = cfg.n_generation_; igen < ngen; ++igen) {
                if (igen < c_max_generation) {
                    {
                        char buf[40];
                        snprintf(buf, sizeof(buf), "x1-space-G%u-a", igen);

                        this->space_storage_[0][igen] = DArena::map(cfg.arena_config_.with_name(std::string(buf)));
                    }
                    {
                        char buf[40];
                        snprintf(buf, sizeof(buf), "x1-space-G%u-b", igen);

                        this->space_storage_[1][igen] = DArena::map(cfg.arena_config_.with_name(std::string(buf)));
                    }

                    this->space_[role::to_space()][igen] = &space_storage_[0][igen];
                    this->space_[role::from_space()][igen] = &space_storage_[1][igen];
                } else {
                    assert(false);
                }
            }

            for (uint32_t igen = cfg.n_generation_; igen < c_max_generation; ++igen) {
                this->space_[role::to_space()][igen] = nullptr;
                this->space_[role::from_space()][igen] = nullptr;
            }
        }

        void
        DX1Collector::visit_pools(const MemorySizeVisitor & visitor) const
        {
            object_types_.visit_pools(visitor);
            root_set_.visit_pools(visitor);

            for (uint32_t j = 0; j < config_.n_generation_; ++j) {
                for (uint32_t i = 0; i < c_n_role; ++i) {
                    space_storage_[i][j].visit_pools(visitor);
                }
            }

            for (uint32_t j = 1; j < config_.n_generation_; ++j) {
                for (uint32_t i = 0; i < c_n_role + 1; ++i) {
                    mlog_storage_[i][j].visit_pools(visitor);
                }
            }
        }

        bool
        DX1Collector::contains(role r, const void * addr) const noexcept
        {
            return !(this->generation_of(r, addr).is_sentinel());
        }

        bool
        DX1Collector::contains_allocated(role r, const void * addr) const noexcept
        {
            Generation g = this->generation_of(r, addr);

            if (g.is_sentinel())
                return false;

            return this->get_space(r, g)->contains_allocated(addr);
        }

        Generation
        DX1Collector::generation_of(role r, const void * addr) const noexcept
        {
            for (Generation gi{0}; gi < config_.n_generation_; ++gi) {
                const DArena * arena = get_space(r, gi);

                if (arena->contains(addr))
                    return gi;
            }

            return Generation::sentinel();
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
                size_t z1 = (d.object_types_.*get_stat_fn)();
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
        DX1Collector::reserved_total() const noexcept
        {
            return accumulate_total_aux(*this, &DArena::reserved);
        }

        size_type
        DX1Collector::size_total() const noexcept
        {
            return committed_total();
        }

        size_type
        DX1Collector::committed_total() const noexcept
        {
            return accumulate_total_aux(*this, &DArena::committed);
        }

        size_type
        DX1Collector::available_total() const noexcept
        {
            return accumulate_total_aux(*this, &DArena::available);
        }

        size_type
        DX1Collector::allocated_total() const noexcept
        {
            return accumulate_total_aux(*this, &DArena::allocated);
        }

        size_type
        DX1Collector::header2size(header_type hdr) const noexcept
        {
            uint32_t z = config_.arena_config_.header_.size(hdr);

            return z;
        }

        object_age
        DX1Collector::header2age(header_type hdr) const noexcept
        {
            uint32_t age = config_.arena_config_.header_.age(hdr);

            assert(age < c_max_object_age);

            return object_age(age);
        }

        uint32_t
        DX1Collector::header2tseq(header_type hdr) const noexcept
        {
            uint32_t tseq = config_.arena_config_.header_.tseq(hdr);

            return tseq;
        }

        bool
        DX1Collector::is_forwarding_header(header_type hdr) const noexcept
        {
            /** forwarding pointer encoded as sentinel tseq **/
            return config_.arena_config_.header_.is_forwarding_tseq(hdr);
        }

        bool
        DX1Collector::is_type_installed(typeseq tseq) const noexcept
        {
            if (object_types_.committed() < sizeof(AGCObject) * (tseq.seqno() + 1))
                return false;

            AGCObject * v = reinterpret_cast<AGCObject *>(object_types_.lo_);

            void * vtable = *(void **)&(v[tseq.seqno()]);

            return (vtable != nullptr);
        }

        AllocInfo
        DX1Collector::alloc_info(value_type mem) const noexcept {
            for (role ri : role::all()) {
                for (Generation gj{0}; gj < config_.n_generation_; ++gj) {
                    const DArena * arena = this->get_space(ri, gj);

                    assert(arena);

                    if (arena->contains(mem)) {
                        return arena->alloc_info(mem);
                    }
                }
            }

            // deliberately attempt on nursery to-space, to capture error info + return sentinel
            return this->get_space(role::to_space(), Generation{0})->alloc_info(mem);
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
                        // pointer validity

                        gco.forward_children(this->ref<ACollector>());
                    }

                    VerifyStats post = verify_stats_;

                    // assert fail -> root contains ptr to from-space
                    assert(pre.n_from_ == post.n_from_);
                }
            }

            // restore run state at end of verify cycle
            this->runstate_ = saved_runstate;

            return true;
        }

        const AGCObject *
        DX1Collector::lookup_type(typeseq tseq) const noexcept
        {
            AGCObject * v = reinterpret_cast<AGCObject *>(object_types_.lo_);

            const AGCObject * target = &(v[tseq.seqno()]);

            assert(reinterpret_cast<const std::byte *>(target) < object_types_.limit_);

            return target;
        }

        /* editor bait: register_type */
        bool
        DX1Collector::install_type(const AGCObject & meta) noexcept
        {
            typeseq tseq = meta._typeseq();

            bool ok = object_types_.expand(sizeof(AGCObject) * (tseq.seqno() + 1));
            if (!ok)
                return false;

            AGCObject * v = reinterpret_cast<AGCObject *>(object_types_.lo_);

            /* explicitly copying vtable pointer here */
            std::memcpy((void*)&(v[tseq.seqno()]), (void*)&meta, sizeof(AGCObject));

            return true;
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
            log && log("step 3a : [STUB] run destructors");
            log && log("step 3b : [STUB] keep reachable weak pointers");

            log && log("step 4  : cleanup");
            this->cleanup_phase(upto);

            if (config_.sanitize_flag_) {
                log && log("step 4b : verify");
                this->verify_ok();
            }
        }

        void
        DX1Collector::swap_roles(Generation upto) noexcept
        {
            scope log(XO_DEBUG(true), xtag("upto", upto));

            for (Generation g = Generation{0}; g < upto; ++g) {
                log && log("swap roles", xtag("g", g));

                std::swap(space_[role::to_space()][g], space_[role::from_space()][g]);
            }
        }

        void
        DX1Collector::cleanup_phase(Generation upto)
        {
            scope log(XO_DEBUG(true), xtag("upto", upto));

            // everything live has been copied out of from-space
            // -> now set to empty
            //
            for (Generation g = Generation{0}; g < upto; ++g) {
                if (config_.sanitize_flag_) {
                    space_[role::from_space()][g]->scrub();
                }

                space_[role::from_space()][g]->clear();
            }

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
                auto self = this->ref<ACollector>();
                from_src.forward_children(self);
                return from_src.data();
            }
        }

        void *
        DX1Collector::_deep_move_interior(void * from_src,
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
            scope log(XO_DEBUG(config_.debug_flag_));

            AllocInfo info = this->alloc_info((std::byte *)from_src);
            AllocHeader hdr = info.header();
            typeseq tseq(info.tseq());

            assert(this->contains_allocated(role::from_space(), from_src));

            if (is_forwarding_header(hdr)) {
                /* already forwarded - pickup destination
                 *
                 * Coordinates with forward_inplace()
                 */
                log && log("disposition: already forwarded");

                return *(void **)from_src;
            }

            /* here: object at from_src not already forwarded */

            if (!this->check_move_policy(hdr, from_src)) {
                /* object at from_src in generation that is not being collected */
                log && log("disposition: not moving from_src");

                return from_src;
            }

            /**
             *  To-space:
             *
             *     to_lo = start of to-space
             *     w,W   = white objects. An object x is white if x
             *             + all immediate children of x are in to-space
             *             (also implies this GC cycle put it there)
             *     g,G   = grey  objects. An object x is gray if it's in to-space,
             *             but possibly has >0 black children
             *     _     = free to-space memory
             *     N     = nursery space (generation{0})
             *     T     = tenured space (generation{1})
             *
             *     wwwwwwwwwwwwwwwwwwwggggggggggggggggggggg_________________...
             *     ^                  ^                    ^
             *     to_lo              grey_lo(N)           free_ptr(N)
             *
             *  After moving children of one object,
             *  advancing {nursery_grey_lo, nursery_free_ptr}
             *
             *     wwwwwwwwwwwwwwwwwwwWWWWgggggggggggggggggGGGGGGGGGGG______...
             *     ^                      ^                           ^
             *     to_lo                  grey_lo(N)                  free_ptr(N)
             *
             *  Invariant:
             *
             *     objects in [to_lo, gray_lo) are white.
             *     all gray objects are in [gray_lo, free_ptr)
             *     memory starting at free_ptr is free.
             *
             *  deep_move terminates when gray_lo catches up to free_ptr
             *
             *  Above is simplified. Complication is that GC (including incremental) may
             *  promote objects from nursery (N) to tenured (T)
             *
             *  So more accurate before/after picture
             *
             *  N  wwwwwwwwwwwwwwwwwwwggggggggggggggggggggg_________________...
             *     ^                  ^                    ^
             *     to_lo(N)           grey_lo(N)           free_ptr(N)
             *
             *  T  wwwwwwwwwwwwwwgggggggggggg_______________________________...
             *     ^             ^           ^
             *     to_lo(T)      grey_lo(T)  free_ptr(N)
             *
             *  After moving children of one object,
             *  advancing {nursery_grey_lo, nursery_free_ptr}
             *
             *  N  wwwwwwwwwwwwwwwwwwwWWWWgggggggggggggggggGGGGGGGGGGG_____...
             *     ^                      ^                           ^
             *     to_lo(N)               grey_lo(N)                  free_ptr(N)
             *
             *  T  wwwwwwwwwwwwwwggggggggggggGGGGG_________________________...
             *     ^             ^                ^
             *     to_lo(T)      grey_lo(T)       free_ptr(T)
             *
             *  deep_move terminates when both:
             *  - gray_lo(N) catches up with free_ptr(N)
             *  - gray_lo(T) catches up with free_ptr(T)
             *
             **/

            log && log("disposition: move subtree");

            /* TODO: AllocIterator pointing to free pointer */
            std::array<std::byte *, c_max_generation> gray_lo_v;
            {
                for (uint32_t g = 0; g < upto; ++g) {
                    gray_lo_v[g] = this->to_space(Generation{g})->free_;
                }
            }

            obj<AAllocator, DX1Collector> alloc(this);
            const AGCObject * iface = lookup_type(tseq);

            assert(iface->_has_null_vptr() == false);

            void * to_dest = this->shallow_move(iface, from_src);

            std::size_t fixup_work = 0;

            /* TODO:
             * - loop here is bad for memory locality
             * - replace with depth-first traversal
             */
            do {
                fixup_work = 0;

                for (Generation g = Generation{0}; g < upto; ++g) {
                    /** object index for this pass **/
                    size_t i_obj = 0;

                    /* TODO: use AllocIterator here */
                    while(gray_lo_v[g] < to_space(g)->free_) {
                        AllocHeader * hdr = (AllocHeader *)gray_lo_v[g];
                        void * src = (hdr + 1);

                        const auto & hdr_cfg = config_.arena_config_.header_;
                        typeseq tseq = typeseq(hdr_cfg.tseq(*hdr));
                        size_t z = hdr_cfg.size_with_padding(*hdr);

                        log && log("deep_move_gc_owned: fwd to-space children",
                                   xtag("g", g),
                                   xtag("i_obj", i_obj),
                                   xtag("src", src),
                                   xtag("tseq", tseq),
                                   xtag("tname", TypeRegistry::id2name(tseq)),
                                   xtag("z", z));

                        const AGCObject * iface = this->lookup_type(tseq);

                        assert(iface->_has_null_vptr() == false);

                        obj<ACollector, DX1Collector> gc(this);

                        iface->forward_children(src, gc);

                        gray_lo_v[g] = ((std::byte *)src) + z;

                        ++i_obj;
                        ++fixup_work;
                    }
                }
            } while (fixup_work > 0);

            log && log(xtag("to_dest", to_dest));

            return to_dest;
        } /*_deep_move_gc_owned*/

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
            if (runstate_.is_running()) {
                // called during collection phase
                this->_forward_inplace_aux(lhs_iface, lhs_data);
            } else if (runstate_.is_verify()) {
                // called during verify_ok
                this->_verify_aux(lhs_iface, lhs_data);
            } else {
                // should be unreachable
                assert(false);
            }
        }

        void
        DX1Collector::_forward_inplace_aux(AGCObject * lhs_iface,
                                           void ** lhs_data)
        {
            scope log(XO_DEBUG(config_.debug_flag_),
                      xtag("lhs_data", lhs_data),
                      xtag("*lhs_data", *lhs_data));

            /* coordinates with DX1Collector::_deep_move() */

            (void)lhs_iface;
            assert(runstate_.is_running());

            /*
             *   lhs   obj<AGCObject>
             *    |    +---------+                +---+-+----+
             *    \--->| .iface  |                | T |G|size| header
             *         +---------+  object_data   +---+-+----+
             *         | .data x----------------->| alloc    |
             *         +---------+                | data     |
             *                                    | for      |
             *                                    | instance |
             *                                    | ...      |
             *                                    +----------+
             */

            void * object_data = (std::byte *)*lhs_data;

            if (!this->contains(role::from_space(), object_data)) {
                /* *lhs_data either:
                 * 1. already in to-space
                 * 2. not in GC-allocated space at all
                 *    (small number of niche examples of this)
                 *
                 * It's important we recognize case (2) up front.
                 * Since not allocated from GC, they don't have
                 * an alloc-header.
                 */
                log && log("disposition: not in from-space");

                return;
            }

            log && log("disposition: in from-space");

            /** NOTE: for form's sake:
             *        lookup actual arena that
             *        allocated object data.
             *        Only using this to get alloc header
             **/
            DArena * some_arena = this->from_space(Generation(0));

            DArena::header_type * p_header
                = some_arena->obj2hdr(object_data);

            DArena::header_type alloc_hdr = *p_header;

            /* recover allocation size */
            std::size_t alloc_z = some_arena->config_.header_.size_with_padding(alloc_hdr);

            if (log) {
                log(xtag("some_arena.lo", some_arena->lo_),
                    xtag("p_header", p_header),
                    xtag("alloc_z", alloc_z));

                AllocInfo info = this->alloc_info((std::byte *)object_data);
                log(xtag("tseq", info.tseq()),
                    xtag("tname", TypeRegistry::id2name(typeseq(info.tseq()))),
                    xtag("is_forwarding_tseq", info.is_forwarding_tseq()),
                    xtag("age", info.age()),
                    xtag("size", info.size()));
            }

            /* need to be able to fit forwarding pointer
             * in place of forwarded object.
             *
             * This is guaranteed anyway, by alignment rules
             */
            assert(alloc_z >= sizeof(uintptr_t));

            if (this->is_forwarding_header(alloc_hdr)) {
                /* *lhs_data already refers to a forwarding pointer */

                /*
                 *   lhs   obj<AGCObject>             (from-space)
                 *    |    +---------+                +---+-+----+
                 *    \--->| .iface  |                |FWD|G|size| alloc_hdr
                 *         +---------+  object_data   +---+-+----+
                 *         | .data x----------------->|     x--------\
                 *         +---------+                |          |   | dest
                 *                                    |          |   |
                 *                                    +----------+   |
                 *                                                   |
                 *                                    (to-space)     |
                 *                                    +---+-+----+   |
                 *                                    |TSQ|G|size|   |
                 *                                    +---+-+----+   |
                 *                                    |          | <-/
                 *                                    |          |
                 *                                    |          |
                 *                                    +----------+
                 */
                void * dest = *(void**)object_data;

                *lhs_data = dest;
                /*
                 *   lhs   obj<AGCObject>
                 *    |    +---------+
                 *    \--->| .iface  |
                 *         +---------+
                 *         | .data x------------\
                 *         +---------+          |
                 *                              | dest
                 *                              |
                 *                              |
                 *                              |     (to-space)
                 *                              |     +---+-+----+
                 *                              |     |TSQ|G|size|
                 *                              |     +---+-+----+
                 *                              \---> |          |
                 *                                    |          |
                 *                                    |          |
                 *                                    +----------+
                 */

                if (log) {
                    log("lhs_data already forwarded", xtag("dest", dest));

                    AllocInfo info = this->alloc_info((std::byte *)dest);
                    log(xtag("tseq", info.tseq()),
                        xtag("tname", TypeRegistry::id2name(typeseq(info.tseq()))),
                        xtag("age", info.age()), xtag("size", info.size()));
                }
            } else if (this->check_move_policy(alloc_hdr, object_data)) {
                /* copy object *lhs + replace with forwarding pointer */

                log && log("forward object now");

                /*
                 *   lhs   obj<AGCObject>             (from-space)
                 *    |    +---------+                +---+-+----+
                 *    \--->| .iface  |                |TSQ|G|size| alloc_hdr
                 *         +---------+  object_data   +---+-+----+
                 *         | .data x----------------> |          |
                 *         +---------+                |          |
                 *                                    |          |
                 *                                    +----------+
                 */

                *lhs_data = this->shallow_move(lhs_iface, *lhs_data);

                /*
                 *   lhs   obj<AGCObject>             (from-space)
                 *    |    +---------+                +---+-+----+
                 *    \--->| .iface  |                |FWD|G|SIZE|
                 *         +---------+                +---+-+----+
                 *         | .data x------------\     |     x--------\
                 *         +---------+          |     |          |   |
                 *                              |     |          |   |
                 *                         dest |     +----------+   |
                 *                              |                    |
                 *                              |     (to-space)     |
                 *                              |     +---+-+----+   |
                 *                              |     |TSQ|G|size|   |
                 *                              |     +---+-+----+   |
                 *                              \---> |          | <-/
                 *                                    |          |
                 *                                    |          |
                 *                                    +----------+
                 */
            } else {
                log && log("object not eligible/required to forward");

                /* object doesn't need to move.
                 * e.g. incremental collection + object is tenured
                 */
            }
        } /*_forward_inplace*/

        void
        DX1Collector::_verify_aux(AGCObject * iface, void * data)
        {
            (void)iface;
            (void)data;

            Generation g = this->generation_of(role::to_space(), data);

            if (g.is_sentinel()) {
                g = this->generation_of(role::from_space(), data);

                if (!g.is_sentinel()) {
                    // verify failure - live pointer still refers to from-space

                    ++(verify_stats_.n_from_);
                } else {
                    ++(verify_stats_.n_ext_);
                }
            } else {
                ++(verify_stats_.n_to_);
            }
        }

        void *
        DX1Collector::shallow_move(const AGCObject * iface, void * from_src)
        {
            scope log(XO_DEBUG(config_.debug_flag_));

            AllocInfo info = this->alloc_info((std::byte *)from_src);
            obj<AAllocator, DX1Collector> alloc(this);

            void * to_dest = iface->shallow_copy(from_src, alloc);

            log && log(xtag("from_src", from_src), xtag("to_dest", to_dest));
            log && log(xtag("tseq", info.tseq()),
                       xtag("tname", TypeRegistry::id2name(typeseq(info.tseq()))),
                       xtag("age", info.age()), xtag("size", info.size()));

            if(to_dest == from_src) {
                assert(false);
            } else {
                *(const_cast<AllocHeader*>(info.p_header_))
                    = AllocHeader(config_
                                  .arena_config_
                                  .header_
                                  .mark_forwarding_tseq(*info.p_header_));

                *(void **)from_src = to_dest;
            }

            return to_dest;
        }

        bool
        DX1Collector::check_move_policy(header_type alloc_hdr,
                                        void * object_data) const noexcept
        {
            (void)object_data;

            // when gc is moving objects, to- and from- spaces have been
            // reversed: forwarding pointers are located in from-space and
            // refer to to-space.

            object_age age = this->header2age(alloc_hdr);

            Generation g = config_.age2gen(age);

            assert(runstate_.is_running());

            return (g < runstate_.gc_upto());
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
                // since a GC cycle that collects the old generation is guarnatted
                // to also collect the young generation.
                return;
            }

            if (src_g == dest_g) {
                // for pointers within the same generation, need to log
                // if source is older than destination.

                const DArena * arena = this->get_space(role::to_space(), src_g);

                const DArena::header_type * src_hdr = arena->obj2hdr(parent);
                const DArena::header_type * dest_hdr = arena->obj2hdr(rhs.data());

                assert(src_hdr && dest_hdr);

                if (header2age(*src_hdr) <= header2age(*dest_hdr)) {
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

            // mlog keyed by generation in which pointer _destination_ resides:
            // collection that moves destination generation around needs to also
            // update pointers such as this one
            //
            MutationLog * mlog = this->mlog_[role::to_space()][dest_g];

            mlog->push_back(MutationLogEntry(parent,
                                             reinterpret_cast<void **>(&(p_lhs->data_)),
                                             rhs));
        }

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

        void
        DX1Collector::reverse_roles(Generation g) noexcept {
            assert(g < config_.n_generation_);

            std::swap(space_[role::from_space()][g], space_[role::to_space()][g]);
        }

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
