/** @file GCObjectStore.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include "GCObjectStore.hpp"
#include "X1Collector.hpp"

#include <xo/object2/Dictionary.hpp>
#include <xo/object2/Array.hpp>
#include <xo/object2/Integer.hpp>
#include <xo/object2/Boolean.hpp>
#include <xo/stringtable2/String.hpp>

#include <xo/arena/DArenaIterator.hpp>

#include <xo/facet/TypeRegistry.hpp>
#include <xo/indentlog/scope.hpp>
#include <cassert>
#include <unistd.h> // for ::getpagesize()

namespace xo {
    using xo::scm::DDictionary;
    using xo::scm::DArray;
    using xo::scm::DString;
    using xo::scm::DInteger;

    using xo::mm::DArena;
    using xo::facet::TypeRegistry;
    using xo::reflect::typeseq;

    namespace mm {

        GCObjectStore::GCObjectStore(const GCObjectStoreConfig & cfg)
            : config_{cfg}
        {
            assert(config_.arena_config_.header_.size_bits_ +
                   config_.arena_config_.header_.age_bits_ +
                   config_.arena_config_.header_.tseq_bits_ <= 64);

            size_t page_z = getpagesize();

            this->_init_object_types(page_z);
            this->_init_space();
        }

        void
        GCObjectStore::_init_object_types(std::size_t page_z)
        {
            /* 1MB reserved address space enough for up to 128k distinct types.
             * In this case don't want to use hugepages since actual #of types
             * likely << .size/8
             */
            this->object_types_
                = ObjectTypeTable::map(ArenaConfig{.name_ = "x1-object-types",
                                                   .size_ = config_.object_types_z_,
                                                   .hugepage_z_ = page_z,
                                                   .store_header_flag_ = false});
        }

        void
        GCObjectStore::_init_space()
        {
            assert(c_n_role == 2);

            for (uint32_t igen = 0, ngen = config_.n_generation_; igen < ngen; ++igen) {
                if (igen < c_max_generation) {
                    {
                        char buf[40];
                        snprintf(buf, sizeof(buf), "x1-space-G%u-a", igen);

                        this->space_storage_[0][igen]
                            = DArena::map(config_.arena_config_.with_name(std::string(buf)));
                    }
                    {
                        char buf[40];
                        snprintf(buf, sizeof(buf), "x1-space-G%u-b", igen);

                        this->space_storage_[1][igen]
                            = DArena::map(config_.arena_config_.with_name(std::string(buf)));
                    }

                    this->space_[role::to_space()][igen] = &space_storage_[0][igen];
                    this->space_[role::from_space()][igen] = &space_storage_[1][igen];
                } else {
                    assert(false);
                }
            }

            for (uint32_t igen = config_.n_generation_; igen < c_max_generation; ++igen) {
                this->space_[role::to_space()][igen] = nullptr;
                this->space_[role::from_space()][igen] = nullptr;
            }

            if (config_.n_generation_ == 2) {
                assert(this->get_space(role::to_space(), Generation{2}) == nullptr);
            }
        }

        bool
        GCObjectStore::is_type_installed(typeseq tseq) const noexcept
        {
            if (tseq.is_sentinel()
                || static_cast<ObjectTypeTable::size_type>(tseq.seqno()) > object_types_.size()) {
                return false;
            }

            const ObjectTypeSlot & slot = object_types_[tseq.seqno()];

            return slot.is_occupied();
        }

        const AGCObject *
        GCObjectStore::lookup_type(typeseq tseq) const noexcept
        {
            scope log(XO_DEBUG(false));

            if (tseq.is_sentinel()
                || (static_cast<ObjectTypeTable::size_type>(tseq.seqno())
                    > object_types_.size())) {

                log.retroactively_enable("out-of-bounds",
                                         xtag("tseq", tseq),
                                         xtag("tname", TypeRegistry::id2name(tseq)));

                log(xtag("types.size", object_types_.size()),
                    xtag("types.allocated", object_types_.store()->allocated()),
                    xtag("types.committed", object_types_.store()->committed()),
                    xtag("types.lo", object_types_.store()->lo_),
                    xtag("types.limit", object_types_.store()->limit_),
                    xtag("types.hi", object_types_.store()->hi_));

                assert(false);
                return nullptr;
            }

            const ObjectTypeSlot & slot = object_types_[tseq.seqno()];

            if (slot.is_null()) {
                log.retroactively_enable("null-vtable",
                                         xtag("tseq", tseq),
                                         xtag("tname", TypeRegistry::id2name(tseq)));

                assert(false);
                return nullptr;
            }

            return slot.iface();
        }

        Generation
        GCObjectStore::generation_of(role r, const void * addr) const noexcept
        {
            for (Generation gi{0}; gi < config_.n_generation_; ++gi) {
                const DArena * arena = this->get_space(r, gi);

                if (arena->contains(addr))
                    return gi;
            }

            return Generation::sentinel();
        }

        auto
        GCObjectStore::header2size(header_type hdr) const noexcept -> size_type
        {
            uint32_t z = config_.arena_config_.header_.size(hdr);

            return z;
        }

        object_age
        GCObjectStore::header2age(header_type hdr) const noexcept
        {
            uint32_t age = config_.arena_config_.header_.age(hdr);

            assert(age < c_max_object_age);

            return object_age(age);
        }

        uint32_t
        GCObjectStore::header2tseq(header_type hdr) const noexcept
        {
            uint32_t tseq = config_.arena_config_.header_.tseq(hdr);

            return tseq;
        }

        bool
        GCObjectStore::is_forwarding_header(header_type hdr) const noexcept
        {
            /** forwarding pointer encoded as sentinel tseq **/
            return config_.arena_config_.header_.is_forwarding_tseq(hdr);
        }

        AllocInfo
        GCObjectStore::alloc_info(value_type mem) const noexcept {
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

        void
        GCObjectStore::visit_pools(const MemorySizeVisitor & visitor) const
        {
            for (uint32_t j = 0; j < config_.n_generation_; ++j) {
                for (uint32_t i = 0; i < c_n_role; ++i) {
                    space_storage_[i][j].visit_pools(visitor);
                }
            }

            object_types_.visit_pools(visitor);
        }

        bool
        GCObjectStore::contains(role r, const void * addr) const noexcept
        {
            return !(this->generation_of(r, addr).is_sentinel());
        }

        bool
        GCObjectStore::contains_allocated(role r, const void * addr) const noexcept
        {
            Generation g = this->generation_of(r, addr);

            if (g.is_sentinel())
                return false;

            return this->get_space(r, g)->contains_allocated(addr);
        }

        bool
        GCObjectStore::report_object_types(obj<AAllocator> mm,
                                           obj<AAllocator> error_mm,
                                           obj<AGCObject> * p_output) const noexcept
        {
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
        }

        bool
        GCObjectStore::report_object_ages(obj<AAllocator> mm,
                                          obj<AAllocator> error_mm,
                                          obj<AGCObject> * p_output) const noexcept
        {
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

        bool
        GCObjectStore::_check_move_policy(header_type alloc_hdr,
                                          void * object_data,
                                          Generation upto) const noexcept
        {
            (void)object_data;

            // when gc is moving objects, to- and from- spaces have been
            // reversed: forwarding pointers are located in from-space and
            // refer to to-space.

            object_age age = this->header2age(alloc_hdr);

            Generation g = config_.age2gen(age);

            //assert(runstate_.is_running());

            return (g < upto);
        }

        void
        GCObjectStore::forward_inplace_aux(DX1Collector * gc,
                                           AGCObject * lhs_iface,
                                           void ** lhs_data,
                                           Generation upto)
        {
            // upto == runstate_.gc_upto()

            scope log(XO_DEBUG(config_.debug_flag_),
                      xtag("lhs_data", lhs_data),
                      xtag("*lhs_data", lhs_data ? *lhs_data : nullptr));

            /* coordinates with DX1Collector::_deep_move() */

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

            if (!object_data) {
                /* trivial to forward nullptr */
                return;
            } else if (!this->contains(role::from_space(), object_data)) {
                /* *lhs_data either:
                 * 1. already in to-space
                 * 2. not in GC-allocated space at all
                 *    (small number of niche examples of this)
                 *
                 * It's important we recognize case (2) up front.
                 * Since not allocated from GC, they don't have
                 * an alloc-header.
                 */
                log && log("disposition: not in from-space. Don't forward, but check children");

                obj<AGCObject> gco(lhs_iface, object_data);
                gco.forward_children(gc->ref<ACollector>());

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
            } else if (this->_check_move_policy(alloc_hdr, object_data, upto)) {
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

                *lhs_data = this->_shallow_move(gc, lhs_iface, *lhs_data);

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
        } /*_forward_inplace_aux*/

        void
        GCObjectStore::swap_roles(Generation upto) noexcept
        {
            scope log(XO_DEBUG(config_.debug_flag_),
                      xtag("upto", upto));

            for (Generation g = Generation{0}; g < upto; ++g) {
                log && log("swap roles", xtag("g", g));

                std::swap(space_[role::to_space()][g], space_[role::from_space()][g]);
            }
        }

        void
        GCObjectStore::cleanup_phase(Generation upto,
                                     bool sanitize_flag)
        {
            scope log(XO_DEBUG(config_.debug_flag_), xtag("upto", upto));

            // everything live has been copied out of from-space
            // -> now set to empty
            //
            for (Generation g = Generation{0}; g < upto; ++g) {
                if (sanitize_flag) {
                    space_[role::from_space()][g]->scrub();
                }

                space_[role::from_space()][g]->clear();
            }
        }

        auto
        GCObjectStore::snap_move_checkpoint(Generation upto) -> GCMoveCheckpoint
        {
            GCMoveCheckpoint gray_lo_v;

            for (uint32_t g = 0; g < upto; ++g) {
                gray_lo_v[g] = this->to_space(Generation{g})->free_;
            }

            return gray_lo_v;
        }

        void
        GCObjectStore::verify_ok(DX1Collector * gc,
                                 X1VerifyStats * p_verify_stats) noexcept
        {
            for (Generation g(0); g < config_.n_generation_; ++g) {
                const DArena * space = this->get_space(role::to_space(), g);

                for (const AllocInfo & info : *space) {

                    if (info.is_forwarding_tseq()) {
                        ++(p_verify_stats->n_fwd_);

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
                            gco.forward_children(gc->ref<ACollector>());
                        } else {
                            ++(p_verify_stats->n_no_iface_);
                            continue;
                        }
                    }
                }
            }
        }

        /* editor bait: register_type */
        bool
        GCObjectStore::install_type(const AGCObject & meta) noexcept
        {
            typeseq tseq = meta._typeseq();

            assert(tseq.seqno() > 0);

            auto ix = static_cast<ObjectTypeTable::size_type>(tseq.seqno());

            if (ix >= object_types_.size()) {
                if (!object_types_.resize(std::max(2 * object_types_.size(), ix + 1)))
                    return false;
            }

            assert(ix < object_types_.size());

            ObjectTypeSlot & slot = object_types_[ix];

            slot.store_iface(&meta);

            return true;
        }

        void *
        GCObjectStore::deep_move_root(DX1Collector * gc,
                                      obj<AGCObject> from_src,
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

            bool src_in_from_space = this->contains(role::from_space(),
                                                    from_src.data());

            if (src_in_from_space) {
                return this->_deep_move_gc_owned(gc, from_src.data(), upto);
            } else {
                // we aren't moving from_src, it's not gc-owned.
                // However weare moving all its gc-owned children

                auto gc_obj = gc->ref<ACollector>();

                GCMoveCheckpoint gray_lo_v
                    = this->snap_move_checkpoint(upto);

                from_src.forward_children(gc_obj);

                // For each generation g:
                //   traverse objects newer than gray_lo_v[g], to make sure children
                //   are forwarded.  Fixpoint reached when gray_lo_v[g] doesn't change.
                //   Remember that forwarding may promote objects to older generation,
                //   so need multiple passes
                //
                this->_forward_children_until_fixpoint(gc, upto, gray_lo_v);

                return from_src.data();
            }
        }

        void *
        GCObjectStore::deep_move_interior(DX1Collector * gc,
                                          void * from_src,
                                          Generation upto)
        {
            scope log(XO_DEBUG(config_.debug_flag_));

            if (!from_src)
                return nullptr;

            bool src_in_from_space = this->contains(role::from_space(), from_src);

            if (!src_in_from_space)
                return from_src;

            return this->_deep_move_gc_owned(gc, from_src, upto);
        }

        void *
        GCObjectStore::_deep_move_gc_owned(DX1Collector * gc,
                                           void * from_src,
                                           Generation upto)
        {
            scope log(XO_DEBUG(config_.debug_flag_));

            AllocInfo info = this->alloc_info((std::byte *)from_src);
            AllocHeader hdr = info.header();
            typeseq tseq(info.tseq());

            assert(this->contains_allocated(role::from_space(), from_src));

            if (this->is_forwarding_header(hdr)) {
                /* already forwarded - pickup destination
                 *
                 * Coordinates with forward_inplace()
                 */
                log && log("disposition: already forwarded");

                return *(void **)from_src;
            }

            /* here: object at from_src not already forwarded */

            if (!this->_check_move_policy(hdr, from_src, upto)) {
                /* object at from_src is in generation that is not being collected */
                log && log("disposition: not moving from_src");

                return from_src;
            }

            log && log("disposition: move subtree");

            /* TODO: AllocIterator pointing to free pointer */
            GCMoveCheckpoint gray_lo_v = this->snap_move_checkpoint(upto);

            //obj<AAllocator, DX1Collector> alloc(this);
            const AGCObject * iface = lookup_type(tseq);

            assert(iface->_has_null_vptr() == false);

            void * to_dest = this->_shallow_move(gc, iface, from_src);

            this->_forward_children_until_fixpoint(gc, upto, gray_lo_v);

            log && log(xtag("to_dest", to_dest));

            return to_dest;
        } /*_deep_move_gc_owned*/

        void *
        GCObjectStore::_shallow_move(DX1Collector * gc,
                                     const AGCObject * iface,
                                     void * from_src)
        {
            scope log(XO_DEBUG(config_.debug_flag_));

            AllocInfo info = this->alloc_info((std::byte *)from_src);

            //obj<AAllocator, DX1Collector> gc_gco(gc);

            void * to_dest = iface->shallow_copy(from_src, gc->ref<AAllocator>());

            log && log(xtag("from_src", from_src), xtag("to_dest", to_dest));
            log && log(xtag("tseq", info.tseq()),
                       xtag("tname", TypeRegistry::id2name(typeseq(info.tseq()))),
                       xtag("age", info.age()),
                       xtag("size", info.size()));

            if (config_.debug_flag_) {
                AllocInfo info_copy = this->alloc_info((std::byte *)to_dest);

                log && log(xtag("age2", info_copy.age()), xtag("size2", info_copy.size()));

                assert((info_copy.age() == config_.arena_config_.header_.max_age())
                       || (info_copy.age() == info.age() + 1));
            }

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
        } /*_shallow_move*/

        void
        GCObjectStore::_forward_children_until_fixpoint(DX1Collector * gc,
                                                        Generation upto,
                                                        GCMoveCheckpoint gray_lo_v)
        {
            scope log(XO_DEBUG(config_.debug_flag_));

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

                        auto gc_gco = gc->ref<ACollector>();

                        iface->forward_children(src, gc_gco);

                        gray_lo_v[g] = ((std::byte *)src) + z;

                        ++i_obj;
                        ++fixup_work;
                    }
                }
            } while (fixup_work > 0);
        } /*_forward_children_until_fixpoint*/

    } /*namespace mm*/
} /*namespace xo*/

/* end GCObjectStore.cpp */
