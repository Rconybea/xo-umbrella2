/** @file AllocFlywheel.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "cx/FacetAppcx.hpp"
#include "handlestore/DHandleStore.hpp"
#include "handlestore/ObjectSlot.hpp"
#include "Top.hpp"
#include <xo/refcnt/Displayable.hpp>

namespace xo::facet {
    using xo::mm::DArenaVector;

    /** @brief memory flywheel for python bindings
     *
     *  Consolidated memory pool and reference set,
     *  for the sake of object handles with automatic
     *  memory management.
     **/
    class AllocFlywheel : public xo::ref::Displayable {
    public:
        using ATop = xo::facet::ATop;
        using HandleStore = DHandleArena<ObjectSlot>;  // was obj<ATop>
        using handle_type = typename HandleStore::handle_type;
        using handle_index_type = typename HandleStore::handle_index_type;
        using PpSink = xo::pp::PpSink;
        using DArena = xo::mm::DArena;
        using ArenaConfig = xo::mm::ArenaConfig;
        using MemorySizeVisitor = xo::mm::MemorySizeVisitor;

    public:
        AllocFlywheel(const FacetAppcx & appcx,
                      DArena && storage,
                      DArenaVector<ObjectSlot> && strong,
                      DArenaVector<handle_index_type> && strong_freelist);

        /** Create new instance from configuration.
         *  We don't ussually heap-allocate.
         *  Exception here because AllocFlywheel may be a
         *  global entry point for python bindings.
         *
         *  @p appcx.  Proof of work (as of Sep2026: facet+indentlog2)
         *  @p storage_cfg.  Configures primary arena.
         *  @p strong_root_cfg.  Configures strong root set.
         *
         *  Free-list arenas are derived from the corresponding root-set configs.
         **/
        static rp<AllocFlywheel> make_app(const FacetAppcx & appcx,
                                          const ArenaConfig & storage_cfg,
                                          const ArenaConfig & strong_root_cfg);

        /** Provide default arguments to make_app() **/
        static rp<AllocFlywheel> make_default_app(const FacetAppcx & appcx);

        FacetAppcx::CreationEvidence facetappcx_creation_evidence() const { return facetappcx_creation_evidence_; }
        Indentlog2Appcx::CreationEvidence indentlog2appcx_creation_evidence() const { return indentlog2appcx_creation_evidence_; }

        DArena & storage() { return store_.storage(); }

        /** report memory consumption, one @ref MemorySizeInfo per pool.
         *
         *  Three pools, in the order @ref HandleStore visits them: the primary
         *  arena objects are allocated from, the strong root set, and the free
         *  list serving that set.  Const: reporting must not be able to disturb
         *  what it measures.
         *
         *  NB reserved/committed/used are three different numbers here.  A root
         *  set reserves its whole configured extent up front and commits as it
         *  grows, so "reserved" says what a flywheel COULD consume and
         *  "committed" what it currently does.
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

        /** this flywheel's state as a wire model -- one animation frame.
         *
         *  See @ref FlywheelInfo for why this is a view model rather than
         *  reflection of the representation.  Const, and allocating only in the
         *  returned value: a frame must not disturb the pools it reports, and
         *  in particular must not allocate from the flywheel's own arena.
         **/
        FlywheelInfo snapshot() const;

        /** count number of non-empty root slots. **/
        handle_index_type strong_root_count() const;

        // from Displayable

        virtual void pretty(PpSink & pp) const override;
        virtual std::string display_string() const override;

    private:
        /** evidence that a FacetAppcx instance was created **/
        FacetAppcx::CreationEvidence facetappcx_creation_evidence_;
        /** evidence that a Indentlog2Appcx instance was created **/
        Indentlog2Appcx::CreationEvidence indentlog2appcx_creation_evidence_;

        /** combined allocator and root set **/
        HandleStore store_;
    };

} /*namespace xo::facet*/

/* end AllocFlywheel.hpp */
