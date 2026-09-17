/** @file DHandleStore.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "../FlywheelInfo.hpp"
#include <xo/arena/DArenaVector.hpp>
#include <xo/arena/DArena.hpp>
#include <stdexcept>
#include <string>

namespace xo::mm {
    /** @tparam Storage provides allocator api with explicit clear.
     *  @tparam Handle represents a generic object reference.
     *  Expect this to be something like obj<ATop> or obj<AGCObject>
     *
     *  Require:
     *  - Storage provides full RAllocator method suite.
     *    Not sure what to do about RAllocator::barrier_assign_aux though
     *  - Storage reports @c config() and @c name().
     *  - Storage was configured with alloc headers enabled, i.e.
     *    @c ArenaConfig::with_store_header_flag(true).  Enforced by the ctor;
     *    see there for why it is a hard requirement rather than a preference
     *  - Handle provides .reset(), putting it into the empty state
     *  - Handle reports ATop's _typeseq() and opaque_data(), for
     *    @ref snapshot.  Only those two: a snapshot must not rotate to another
     *    facet, or it would work for some representations and throw for
     *    others.
     *  - Handle is contextually convertible to bool, false iff empty.
     *    Slot integrity depends on this. It is how release distinguishes
     *    {occupied, empty} slots.
     **/
    template <typename Storage,
              typename Handle>
    class DHandleStore {
    public:
        /** @defgroup mm-handlestore-types **/
        ///@{
        using handle_type = Handle;
        using handle_index_type = typename DArenaVector<Handle>::size_type;
        using typeseq = typename Storage::typeseq;
        using size_type = typename Storage::size_type;
        using value_type = typename Storage::value_type;
        using range_type = typename Storage::range_type;
        ///@}

    public:
        /** @defgroup mm-handlestore-ctors **/
        ///@{

        DHandleStore() = default;
        /** @p strong_freelist and @p weak_freelist hold the index positions of
         *  empty slots in @p strong / @p weak respectively;
         *  in no particular order.
         **/
        DHandleStore(Storage && storage,
                     DArenaVector<Handle> && strong,
                     DArenaVector<handle_index_type> && strong_freelist)
        : storage_{std::move(storage)},
          strong_refs_{std::move(strong)},
          strong_freelist_{std::move(strong_freelist)}
        {
            /* Alloc headers are REQUIRED, not merely useful.
             *
             * DArena::alloc_info() reads the header that store_header_flag_
             * controls.  Without it the read is not a graceful failure -- it
             * SEGFAULTS (measured 2026-09-15), and alloc_info() is reachable
             * from ~10 sites including RAllocator::alloc_info and
             * DArenaIterator.  So a headerless storage arena is a crash waiting
             * for whoever first asks an allocation how big it is.
             *
             * Checked here rather than left to AllocFlywheel::make_app, which
             * also sets it: this is the class that owns the invariant, and a
             * direct construction must not be able to skip it.
             *
             * Cost, for the record: 8 bytes per allocation, and every offset
             * shifts (a 16-byte object goes from 0,16 to 8,32).
             */
            if (!storage_.config().store_header_flag_) {
                throw std::runtime_error
                    (std::string("DHandleStore: storage arena must have alloc"
                                 " headers enabled -- see"
                                 " ArenaConfig::with_store_header_flag; arena=")
                     + std::string(this->name()));
            }
        }

        /** move-assignment **/
        DHandleStore & operator=(DHandleStore && other) = default;

        ///@}

        /** @defgroup mm-handlestore-access-methods **/
        ///@{

        /** false -> not eligible for GC (allocates own memory + not moveable) **/
        static constexpr bool is_gc_eligible() { return false; }

        std::string_view name() const noexcept { return storage_.name(); }
        size_type reserved() const noexcept { return storage_.reserved(); }
        size_type size() const noexcept { return storage_.size(); }
        size_type committed() const noexcept { return storage_.committed(); }
        size_type available() const noexcept { return storage_.available(); }
        size_type allocated() const noexcept { return storage_.allocated(); }
        /** one entry per pool, in the order below. **/
        void visit_pools(const MemorySizeVisitor & fn) const {
            storage_.visit_pools(fn);
            strong_refs_.visit_pools(fn);
            strong_freelist_.visit_pools(fn);
        }
        /** enumerates the same vectors as visit_pools() **/
        bool contains(const void * p) const noexcept {
            return (storage_.contains(p)
                    || strong_refs_.contains(p) || strong_freelist_.contains(p));
        }
        AllocError last_error() const noexcept { return storage_.last_error(); }
        AllocInfo alloc_info(value_type mem) const noexcept { return storage_.alloc_info(mem); }
        range_type alloc_range(DArena & mm) const noexcept { return storage_.alloc_range(mm); }

        ///@}

        /** @defgroup mm-handlestore-mutable-methods **/
        ///@{

        Storage & storage() { return storage_; }
        /* const overload: a snapshot reads bounds and must not be able to
         * disturb what it measures
         */
        const Storage & storage() const { return storage_; }

        bool expand(size_type z) { return storage_.expand(z); }
        value_type alloc(typeseq tseq, size_type z) noexcept { return storage_.alloc(tseq, z); }
        value_type super_alloc(typeseq tseq, size_type z) noexcept { return storage_.super_alloc(tseq, z); }
        value_type sub_alloc(size_type z, bool complete_flag) noexcept { return storage_.sub_alloc(z, complete_flag); }
        value_type alloc_copy(value_type src) noexcept { return storage_.alloc_copy(src); }

        /** copy handle @p x into strong reference set.
         *  @return pair (i, &h), where i indexed &h in strong reference set,
         *  and h is a copy of @p x.
         *
         *  Require: @p x refers to memory owned by @ref storage_
         **/
        std::pair<handle_index_type, Handle*> add_strong_ref(Handle x) {
            return _add_ref(strong_refs_, strong_freelist_, x);
        }

        /** release the strong slot at @p ix, returning it to the free list.
         *  Idempotent.
         **/
        void remove_strong_ref(size_type ix) {
            _remove_ref(strong_refs_, strong_freelist_, ix);
        }

        /** this store's state as a wire model -- see @ref FlywheelInfo.
         *
         *  Written HERE, beside the members it reports, rather than in a helper
         *  above.  The hazard of a view model is that it drifts from the thing
         *  it describes; the mitigation is that whoever adds a member to this
         *  class reads this in the same file.
         *
         *  Reports only OCCUPIED slots, so a frame is proportional to what is
         *  rooted rather than to capacity -- a default flywheel has 256 slots
         *  and typically holds a handful.
         *
         *  @p base is the address @ref SlotInfo::offset_ is measured from --
         *  the storage arena's low bound.  Passed in rather than read from
         *  @ref storage_, which is a template parameter here and need not
         *  expose bounds.
         *
         *  Fills @ref SlotInfo::typeseq_ but NOT @ref SlotInfo::type_: naming a
         *  typeseq needs xo-facet's TypeRegistry, and this class is generic
         *  over Handle.  @ref AllocFlywheel::snapshot fills the names in.
         **/
        void snapshot(RootSetInfo * p_out, std::uint64_t base) const {
            p_out->size_ = static_cast<std::uint32_t>(strong_refs_.size());
            p_out->capacity_ = static_cast<std::uint32_t>(strong_refs_.capacity());
            p_out->live_ = static_cast<std::uint32_t>(this->strong_root_count());

            p_out->free_.clear();
            p_out->free_.reserve(strong_freelist_.size());
            for (std::size_t i = 0, n = strong_freelist_.size(); i < n; ++i)
                p_out->free_.push_back(static_cast<std::uint32_t>(strong_freelist_[i]));

            p_out->slot_v_.clear();
            p_out->slot_v_.reserve(p_out->live_);
            for (std::size_t i = 0, n = strong_refs_.size(); i < n; ++i) {
                const Handle & h = strong_refs_[i];

                if (!h)
                    continue;

                SlotInfo slot;
                slot.ix_ = static_cast<std::uint32_t>(i);
                slot.typeseq_ = h._typeseq().seqno();
                /* every rooted object was allocated from this flywheel's own
                 * arena -- make_strong_ref requires it -- so the subtraction
                 * cannot underflow for a well-formed store
                 */
                slot.offset_ = reinterpret_cast<std::uint64_t>(h.opaque_data()) - base;

                p_out->slot_v_.push_back(std::move(slot));
            }
        }

        /** counts non-empty strong slots **/
        handle_index_type strong_root_count() const {
            return strong_refs_.size() - strong_freelist_.size();
        }

        void clear() {
            // 1. clears refs + freelists.
            strong_refs_.clear();
            strong_freelist_.clear();
            // 2. clear storage
            storage_.clear();
        }

        ///@}

    private:
        /** @defgroup mm-handlestore-impl-methods **/
        ///@{

        /** reuse a released slot if there is one, else grow the vector **/
        static std::pair<handle_index_type, Handle*>
        _add_ref(DArenaVector<Handle> & refs,
                 DArenaVector<handle_index_type> & freelist,
                 Handle x)
        {
            if (!freelist.empty()) {
                handle_index_type ix = freelist.back();

                freelist.pop_back();
                refs[ix] = x;

                return std::make_pair(ix, &refs[ix]);
            }

            auto ix = refs.size();

            /* nullptr when the vector is full: a DArenaVector fixes capacity at
             * construction, so this is how root-set exhaustion reaches a caller
             */
            Handle * ref = refs.push_back(x);

            return std::make_pair(ix, ref);
        }

        /** reset a slot, and if it was non-empty return to freelist **/
        static void _remove_ref(DArenaVector<Handle> & refs,
                                DArenaVector<handle_index_type> & freelist,
                                size_type ix)
        {
            if (ix >= refs.size())
                return;

            if (refs[ix]) {
                refs[ix].reset();

                /* cannot fail: freelist capacity >= refs capacity, and at most
                 * every slot is free at once
                 */
                freelist.push_back(ix);
            } else {
                /* already released.  See remove_strong_ref's note on what this
                 * does and does not protect
                 */
            }
        }

        ///@}

    private:

        /** @defgroup mm-handlestore-instance-vars **/
        ///@{

        /** Storage. Must satisfy allocator api **/
        Storage storage_;

        /** Strong references.
         *
         *  Promise: memory allocated from @ref storage_ remains
         *  valid while any non-null references remain in @ref strong_
         **/
        DArenaVector<Handle> strong_refs_;

        /** Index positions of empty slots in @ref strong_refs_ **/
        DArenaVector<handle_index_type> strong_freelist_;

        ///@}
    };

    template <typename Handle>
    using DHandleArena = DHandleStore<DArena, Handle>;

} /*namespace xo::mm*/

/* end DHandleStore.hpp */
