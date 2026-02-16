/** @file DX1Collector.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "Allocator.hpp"
#include "detail/IAllocator_DX1Collector.hpp"
#include "detail/ICollector_DX1Collector.hpp"
#include "arena/IAllocator_DArena.hpp"
#include "xo/gc/DX1Collector.hpp"
#include "xo/gc/DX1CollectorIterator.hpp"
#include "generation.hpp"
#include "object_age.hpp"
#include <xo/facet/obj.hpp>
#include <xo/indentlog/scope.hpp>
#include <cassert>
#include <cstdint>
#include <sys/mman.h>
#include <unistd.h> // for ::getpagesize()

namespace xo {
    using xo::mm::AAllocator;
    using xo::facet::typeseq;
    using xo::facet::with_facet;

    namespace mm {

        X1CollectorConfig
        X1CollectorConfig::with_name(std::string name)
        {
            X1CollectorConfig copy = *this;
            copy.name_ = std::move(name);
            return copy;
        }

        X1CollectorConfig
        X1CollectorConfig::with_size(std::size_t gen_z)
        {
            X1CollectorConfig copy = *this;
            copy.arena_config_ = arena_config_.with_size(gen_z);
            return copy;
        }

        // ----- GCRunState -----

        GCRunState::GCRunState(generation gc_upto)
            : gc_upto_{gc_upto}
        {}

        GCRunState
        GCRunState::gc_not_running()
        {
            return GCRunState(generation(0));
        }

        GCRunState
        GCRunState::gc_upto(generation g)
        {
            return GCRunState(generation(g + 1));
        }

        // ----- DX1Collector -----

        using size_type = xo::mm::DX1Collector::size_type;

        DX1Collector::DX1Collector(const X1CollectorConfig & cfg) : config_{cfg}
        {
            assert(config_.arena_config_.header_.size_bits_ +
                   config_.arena_config_.header_.age_bits_ +
                   config_.arena_config_.header_.tseq_bits_ <= 64);

            size_t page_z = getpagesize();

            /* 1MB reserved address space enough for up to 128k distinct types.
             * In this case don't want to use hugepages since actual #of types
             * likely << .size/8
             */
            object_types_ = DArena::map(
                ArenaConfig{
                    .name_ = "x1-object-types",
                    .size_ = cfg.object_types_z_,
                    .hugepage_z_ = page_z,
                    .store_header_flag_ = false});

            roots_ = DArena::map(
                ArenaConfig{
                    .name_ = "x1-object-roots",
                    .size_ = cfg.object_roots_z_,
                    .hugepage_z_ = page_z,
                    .store_header_flag_ = false});

            for (uint32_t igen = 0, ngen = cfg.n_generation_; igen < ngen; ++igen) {
                {
                    char buf[40];
                    snprintf(buf, sizeof(buf), "x1-space-G%u-a", igen);

                    space_storage_[0][igen] = DArena::map(cfg.arena_config_.with_name(std::string(buf)));
                }
                {
                    char buf[40];
                    snprintf(buf, sizeof(buf), "x1-space-G%u-b", igen);

                    space_storage_[1][igen] = DArena::map(cfg.arena_config_.with_name(std::string(buf)));
                }

                space_[role::to_space()][igen] = &space_storage_[0][igen];
                space_[role::from_space()][igen] = &space_storage_[1][igen];
            }

            for (uint32_t igen = cfg.n_generation_; igen < c_max_generation; ++igen) {
                space_[role::to_space()][igen] = nullptr;
                space_[role::from_space()][igen] = nullptr;
            }
        }

        void
        DX1Collector::visit_pools(const MemorySizeVisitor & visitor) const
        {
            object_types_.visit_pools(visitor);
            roots_.visit_pools(visitor);

            for (uint32_t i = 0; i < c_n_role; ++i) {
                for (uint32_t j = 0; j < config_.n_generation_; ++j) {
                    space_storage_[i][j].visit_pools(visitor);
                }
            }
        }

        bool
        DX1Collector::contains(role r, const void * addr) const noexcept
        {
            for (generation gi{0}; gi < config_.n_generation_; ++gi) {
                const DArena * arena = get_space(r, gi);

                if (arena->contains(addr))
                    return true;
            }

            return false;
        }

        AllocError
        DX1Collector::last_error() const noexcept
        {
            // TODO:
            // need to adjust here if runtime errors
            // encountered during gc.

            return get_space(role::to_space(), generation::nursery())->last_error_;
        }

        namespace {
            size_type
            accumulate_total_aux(const DX1Collector & d,
                                 size_t (DArena::* get_stat_fn)() const) noexcept
            {
                size_t z1 = (d.object_types_.*get_stat_fn)();
                size_t z2 = (d.roots_.*get_stat_fn)();

                size_t z3 = 0;

                for (role ri : role::all()) {
                    for (generation gj{0}; gj < d.config_.n_generation_; ++gj) {
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

        const AGCObject *
        DX1Collector::lookup_type(typeseq tseq) const noexcept
        {
            AGCObject * v = reinterpret_cast<AGCObject *>(object_types_.lo_);

            return &(v[tseq.seqno()]);
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
            std::byte * mem
                = roots_.alloc(typeseq::sentinel(),
                               sizeof(obj<AGCObject>*));
            assert(mem);

            *(obj<AGCObject> **)mem = p_root;
        }

        void
        DX1Collector::request_gc(generation upto) noexcept
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
        DX1Collector::execute_gc(generation upto) noexcept
        {
            scope log(XO_DEBUG(true), xtag("upto", upto));

            assert(!runstate_.is_running());

            //auto t0 = std::chrono::steady_clock::now();

            log && log("step 0a : update run state");
            this->runstate_ = GCRunState::gc_upto(upto);

            log && log("step 0a : [STUB] snapshot alloc state");

            log && log("step 0b : [STUB] scan for object statistics");

            log && log("step 1  : swap from/to roles");
            this->swap_roles(upto);

            log && log(xtag("from_0", get_space(role::from_space(), generation{0})->lo_),
                       xtag("to_0", get_space(role::to_space(), generation{0})->lo_));

            log && log("step 2a : copy roots");
            this->copy_roots(upto);

            log && log("step 2b : [STUB] copy pinned");
            log && log("step 3a : [STUB] run destructors");
            log && log("step 3b : [STUB] keep reachable weak pointers");
            log && log("step 4  : [STUB] cleanup");
        }

        void
        DX1Collector::swap_roles(generation upto) noexcept
        {
            scope log(XO_DEBUG(true), xtag("upto", upto));

            for (generation g = generation{0}; g < upto; ++g) {
                log && log("swap roles", xtag("g", g));

                std::swap(space_[role::to_space()][g], space_[role::from_space()][g]);
            }
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
        DX1Collector::deep_move(void * from_src, generation upto)
        {
            scope log(XO_DEBUG(config_.debug_flag_));

            if (!from_src)
                return nullptr;

            if (!this->contains(role::from_space(), from_src)) {
                /* presumeably memory not owned by collector
                 * (e.g. DBoolean {true, false}, DUniqueString {owned by StringTable} etc.)
                 */
                return from_src;
            }

            AllocInfo info = this->alloc_info((std::byte *)from_src);
            AllocHeader hdr = info.header();
            typeseq tseq(info.tseq());

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
                    gray_lo_v[g] = this->to_space(generation{g})->free_;
                }
            }

            obj<AAllocator, DX1Collector> alloc(this);
            const AGCObject * iface = lookup_type(tseq);
            void * to_dest = this->shallow_move(iface, from_src);

            std::size_t fixup_work = 0;

            /* TODO:
             * - loop here is bad for memory locality
             * - replace with depth-first traversal
             */
            do {
                fixup_work = 0;

                for (generation g = generation{0}; g < upto; ++g) {
                    /* TODO: use AllocIterator here */
                    while(gray_lo_v[g] < to_space(g)->free_) {
                        AllocHeader * hdr = (AllocHeader *)gray_lo_v[g];
                        void * src = (hdr + 1);

                        log && log("fwd children", xtag("src", src));

                        const auto & hdr_cfg = config_.arena_config_.header_;
                        typeseq tseq = typeseq(hdr_cfg.tseq(*hdr));
                        size_t z = hdr_cfg.size_with_padding(*hdr);

                        const AGCObject * iface = this->lookup_type(tseq);
                        obj<ACollector, DX1Collector> gc(this);

                        iface->forward_children(src, gc);

                        gray_lo_v[g] = ((std::byte *)src) + z;
                        ++fixup_work;
                    }
                }
            } while (fixup_work > 0);

            log && log(xtag("to_dest", to_dest));

            return to_dest;
        }

        void
        DX1Collector::copy_roots(generation upto) noexcept
        {
            scope log(XO_DEBUG(true));

            for (obj<AGCObject> ** p_root = (obj<AGCObject> **)roots_.lo_;
                 p_root < (obj<AGCObject> **)roots_.free_; ++p_root)
            {
                log && log("copy root", xtag("**p_root.data.pre", (**p_root).data_));

                (*p_root)->reset_opaque(this->deep_move((*p_root)->data_, upto));

                log && log(xtag("**p_root.data.post", (**p_root).data_));
            }
        }

        void
        DX1Collector::forward_inplace(AGCObject * lhs_iface,
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
            DArena * some_arena = this->from_space(generation(0));

            DArena::header_type * p_header
                = some_arena->obj2hdr(object_data);

            DArena::header_type alloc_hdr = *p_header;

            /* recover allocation size */
            std::size_t alloc_z = some_arena->config_.header_.size_with_padding(alloc_hdr);

            log && log(xtag("some_arena.lo", some_arena->lo_),
                       xtag("p_header", p_header),
                       xtag("alloc_z", alloc_z));

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
                 *                                    |FWD|G|size|<--/
                 *                                    +---+-+----+
                 *                                    |          |
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
                 *                              \---->|FWD|G|size|
                 *                                    +---+-+----+
                 *                                    |          |
                 *                                    |          |
                 *                                    |          |
                 *                                    +----------+
                 */
            } else if (this->check_move_policy(alloc_hdr, object_data)) {
                /* copy object *lhs + replace with forwarding pointer */

                /*
                 *   lhs   obj<AGCObject>             (from-space)
                 *    |    +---------+                +---+-+----+
                 *    \--->| .iface  |                |FWD|G|size| alloc_hdr
                 *         +---------+  object_data   +---+-+----+
                 *         | .data x----------------->|          |
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
                 *                              \---->|FWD|G|size|<--/
                 *                                    +---+-+----+
                 *                                    |          |
                 *                                    |          |
                 *                                    |          |
                 *                                    +----------+
                 */
            } else {
                /* object doesn't need to move.
                 * e.g. incremental collection + object is tenured
                 */
            }
        } /*forward_inplace*/

        void *
        DX1Collector::shallow_move(const AGCObject * iface, void * from_src)
        {
            scope log(XO_DEBUG(config_.debug_flag_));

            AllocInfo info = this->alloc_info((std::byte *)from_src);
            obj<AAllocator, DX1Collector> alloc(this);

            void * to_dest = iface->shallow_copy(from_src, alloc);

            log && log(xtag("from_src", from_src), xtag("to_dest", to_dest));

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

            generation g = config_.age2gen(age);

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
            if (with_facet<AAllocator>::mkobj(to_space(generation{0})).expand(z))
                return with_facet<AAllocator>::mkobj(from_space(generation{0})).expand(z);

            return false;
        }

        AllocInfo
        DX1Collector::alloc_info(value_type mem) const noexcept {
            for (role ri : role::all()) {
                for (generation gj{0}; gj < config_.n_generation_; ++gj) {
                    const DArena * arena = this->get_space(ri, gj);

                    assert(arena);

                    if (arena->contains(mem)) {
                        return arena->alloc_info(mem);
                    }
                }
            }

            // deliberately attempt on nursery to-space, to capture error info + return sentinel
            return this->get_space(role::to_space(), generation{0})->alloc_info(mem);
        }

        DX1CollectorIterator
        DX1Collector::begin() const noexcept
        {
            scope log(XO_DEBUG(false));

            const DArena * arena
                = get_space(role::to_space(),
                            generation{0});

            return DX1CollectorIterator(this,
                                        generation{0},
                                        generation{config_.n_generation_},
                                        arena->begin(),
                                        arena->end());
        }

        DX1CollectorIterator
        DX1Collector::end() const noexcept {
            scope log(XO_DEBUG(false));

            generation gen_hi = generation{config_.n_generation_};

            /** valid iterator for end points to end of last DArena.
             *  otherwise will interfere with working compare
             *  (since invalid iterators are incomparable)
             **/

            const DArena * arena
                = get_space(role::to_space(),
                            generation(config_.n_generation_ - 1));
            DArenaIterator arena_end = arena->end();

            return DX1CollectorIterator(this,
                                        gen_hi,
                                        gen_hi,
                                        arena_end,
                                        arena_end);
        }

        void
        DX1Collector::reverse_roles(generation g) noexcept {
            assert(g < config_.n_generation_);

            std::swap(space_[role::from_space()][g], space_[role::to_space()][g]);
        }

        void
        DX1Collector::clear() noexcept {
            for (role ri : role::all()) {
                for (generation gj{0}; gj < config_.n_generation_; ++gj) {
                    DArena * arena = this->get_space(ri, gj);

                    assert(arena);

                    arena->clear();
                }
            }
        }
    } /*namespace mm*/
} /*namespace xo*/

/* end DX1Collector.cpp */
