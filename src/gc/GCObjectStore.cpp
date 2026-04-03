/** @file GCObjectStore.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include "GCObjectStore.hpp"

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
        GCObjectStore::_shallow_move(obj<AAllocator> mm,
                                     const AGCObject * iface,
                                     void * from_src)
        {
            scope log(XO_DEBUG(config_.debug_flag_));

            AllocInfo info = this->alloc_info((std::byte *)from_src);

            //obj<AAllocator, DX1Collector> gc_gco(gc);

            void * to_dest = iface->shallow_copy(from_src, mm);

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
        } /*shallow_move*/

#ifdef MARKED
        void
        GCObjectStore::_forward_children_until_fixpoint(DX1Collector * gc,
                                                        Generation upto,
                                                        const GCMoveCheckpoint & gray_lo_v)
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

                        auto gc = this->ref<ACollector>();

                        iface->forward_children(src, gc);

                        gray_lo_v[g] = ((std::byte *)src) + z;

                        ++i_obj;
                        ++fixup_work;
                    }
                }
            } while (fixup_work > 0);
        } /*_forward_children_until_fixpoint*/
#endif


    } /*namespace mm*/
} /*namespace xo*/

/* end GCObjectStore.cpp */
