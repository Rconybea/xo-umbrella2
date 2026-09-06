/** @file AllocFlywheel.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

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

    public:
        AllocFlywheel(DArena && storage,
                      DArenaVector<obj<ATop>> && strong,
                      DArenaVector<obj<ATop>> && weak);

        /** Create new instance from configuration.
         *  We don't ussually heap-allocate. Exception here because
         *  AllocFlywheel may be a global entry point for python bindings.
         *
         *  @p storage_cfg.  Configures primary arena.
         *  @p strong_root_cfg.  Configures strong root set.
         *  @p weak_root_cfg.  Configures weak root set.
         **/
        static rp<AllocFlywheel> make_app(ArenaConfig & storage_cfg,
                                          ArenaConfig & strong_root_cfg,
                                          ArenaConfig & weak_root_cfg);

        DArena & storage() { return store_.storage(); }

        /** insert strong reference to @p x into this flywheel **/
        std::pair<handle_index_type, handle_type*> add_strong_ref(handle_type x);

        // from Displayable

        virtual void pretty(PpSink & pp) const override;
        virtual std::string display_string() const override;

    private:
        /** combined allocator and root set **/
        HandleStore store_;
    };

} /*namespace xo::mm*/

/* end AllocFlywheel.hpp */
