/** @file AllocFlywheel.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "cx/FacetAppcx.hpp"
#include "handlestore/DHandleStore.hpp"
#include "Top.hpp"
#include <xo/refcnt/Displayable.hpp>

namespace xo::mm {
    /** @brief memory flywheel for python bindings
     *
     *  Consolidated memory pool and reference set,
     *  for the sake of object handles with automatic
     *  memory management.
     **/
    class AllocFlywheel : public xo::ref::Displayable {
    public:
        using ATop = xo::facet::ATop;
        using HandleStore = DHandleArena<obj<ATop>>;
        using handle_type = typename HandleStore::handle_type;
        using handle_index_type = typename HandleStore::handle_index_type;
        using PpSink = xo::pp::PpSink;
        using MemorySizeVisitor = xo::mm::MemorySizeVisitor;

    public:
        AllocFlywheel(const FacetAppcx & appcx,
                      DArena && storage,
                      DArenaVector<obj<ATop>> && strong,
                      DArenaVector<handle_index_type> && strong_freelist,
                      DArenaVector<obj<ATop>> && weak,
                      DArenaVector<handle_index_type> && weak_freelist);

        /** Create new instance from configuration.
         *  We don't ussually heap-allocate.
         *  Exception here because AllocFlywheel may be a
         *  global entry point for python bindings.
         *
         *  @p appcx.  Proof of work (as of Sep2026: facet+indentlog2)
         *  @p storage_cfg.  Configures primary arena.
         *  @p strong_root_cfg.  Configures strong root set.
         *  @p weak_root_cfg.  Configures weak root set.
         *
         *  Free-list arenas are derived from the corresponding root-set configs.
         **/
        static rp<AllocFlywheel> make_app(const FacetAppcx & appcx,
                                          const ArenaConfig & storage_cfg,
                                          const ArenaConfig & strong_root_cfg,
                                          const ArenaConfig & weak_root_cfg);

        /** Provide default arguments to make_app() **/
        static rp<AllocFlywheel> make_default_app(const FacetAppcx & appcx);

        const FacetAppcx & facet_appcx() const { return facet_appcx_; }

        /** forwarded from @ref facet_appcx_, so anything holding a flywheel is
         *  one hop from the xo-indentlog2 context too -- see
         *  @ref xo::carries_indentlog2.  Without this the chain is transitive
         *  and a caller has to know to make two hops.
         **/
        const Indentlog2Appcx & indentlog2_appcx() const {
            return facet_appcx_.indentlog2_appcx();
        }

        DArena & storage() { return store_.storage(); }

        /** report memory consumption, one @ref MemorySizeInfo per pool.
         *
         *  Memory pools pools owned by this flyswheel.
         **/
        void visit_pools(const MemorySizeVisitor & fn) const { store_.visit_pools(fn); }

        /** insert strong reference to @p x into this flywheel **/
        std::pair<handle_index_type, handle_type*> add_strong_ref(handle_type x);

        /** release the strong slot at @p ix, returning it for reuse.
         *
         *  Called by ~ObjectHandleBase, so dropping the last python reference
         *  to a handle unpins its object.  Idempotent per index; see
         *  DHandleStore::remove_strong_ref for what that does and does not
         *  protect.
         **/
        void remove_strong_ref(handle_index_type ix);
        void remove_weak_ref(handle_index_type ix);

        /** count number of non-empty root slots. **/
        handle_index_type strong_root_count() const;
        handle_index_type weak_root_count() const;

        // from Displayable

        virtual void pretty(PpSink & pp) const override;
        virtual std::string display_string() const override;

    private:
        /** xo-facet/ context **/
        const FacetAppcx & facet_appcx_;

        /** combined allocator and root set **/
        HandleStore store_;
    };

} /*namespace xo::mm*/

/* end AllocFlywheel.hpp */
