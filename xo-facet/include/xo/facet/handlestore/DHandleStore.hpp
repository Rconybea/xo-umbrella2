/** @file DHandleStore.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include <xo/arena/DArenaVector.hpp>
#include <xo/arena/DArena.hpp>

namespace xo::mm {
    /** @tparam Storage provides allocator api with explicit clear.
     *  @tparam Handle represents a generic object reference.
     *  Expect this to be something like obj<ATop> or obj<AGCObject>
     *
     *  Require:
     *  - Storage provides full RAllocator method suite.
     *    Not sure what to do about RAllocator::barrier_assign_aux though
     *  - Handle provides .reset(), putting it into the empty state
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
        {}

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
