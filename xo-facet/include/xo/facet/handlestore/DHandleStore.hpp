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
     *  - Handle provides .clear()
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
        DHandleStore(Storage && storage,
                     DArenaVector<Handle> && strong,
                     DArenaVector<Handle> && weak)
        : storage_{std::move(storage)},
          strong_refs_{std::move(strong)},
          weak_refs_{std::move(weak)} {}

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
        void visit_pools(const MemorySizeVisitor & fn) const {
            storage_.visit_pools(fn);
            strong_refs_.visit_pools(fn);
            weak_refs_.visit_pools(fn);
        }
        bool contains(const void * p) const noexcept {
            return storage_.contains(p) || strong_refs_.contains(p) || weak_refs_.contains(p);
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
            auto ix = strong_refs_.size();

            Handle * ref = strong_refs_.push_back(x);

            return std::make_pair(ix, ref);
        }

        /** copy handle @p x into weak reference set.
         *  @return pair (i, &h), where i indexed &h in weak reference set,
         *  and h is a copy of @p x.
         *
         *  Require: @p x refers to memory owned by @ref storage_
         **/
        std::pair<handle_index_type, Handle*> add_weak_ref(Handle x) {
            auto ix = weak_refs_.size();

            Handle * ref = weak_refs_.push_back(x);

            return std::make_pair(ix, ref);
        }

        void remove_strong_ref(size_type ix) {
            if (ix < strong_refs_.size()) {
                strong_refs_[ix].clear();
            }
        }

        void remove_weak_ref(size_type ix) {
            if (ix < weak_refs_.size()) {
                weak_refs_[ix].clear();
            }
        }

        void clear() {
            // 1. clear refs
            strong_refs_.clear();
            weak_refs_.clear();
            // 2. clear storage
            storage_.clear();
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

        /** Weak references.
         *
         *  Promise: these do not keep @ref storage_ alive,
         *  but will be sent to a well-defined sentinel state
         *  whenever @ref storage_ is reclaimed/cleared.
         **/
        DArenaVector<Handle> weak_refs_;

        ///@}
    };

    template <typename Handle>
    using DHandleArena = DHandleStore<DArena, Handle>;

} /*namespace xo::mm*/

/* end DHandleStore.hpp */
