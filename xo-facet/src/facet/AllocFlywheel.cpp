/** @file AllocFlywheel.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "AllocFlywheel.hpp"
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/pretty_struct.hpp>

namespace xo::mm {
    AllocFlywheel::AllocFlywheel(const FacetAppcx & facet_appcx,
                                 DArena && storage,
                                 DArenaVector<obj<ATop>> && strong,
                                 DArenaVector<obj<ATop>> && weak)
      : facet_appcx_{facet_appcx},
        store_{std::move(storage), std::move(strong), std::move(weak)}
    {
        // facet_appcx_: proof of work: facet,indentlog2 init performed;
        // implies config-dependent globals setup, including:
        // - FacetRegistry
        // - TempArena
        // - TempPrettySink
        // - SinkFactory
    }

    rp<AllocFlywheel>
    AllocFlywheel::make_app(const FacetAppcx & appcx,
                            const ArenaConfig & storage_cfg,
                            const ArenaConfig & strong_cfg,
                            const ArenaConfig & weak_cfg)
    {
        return new AllocFlywheel(appcx,
                                 DArena::map(storage_cfg),
                                 DArenaVector<obj<ATop>>::map(strong_cfg),
                                 DArenaVector<obj<ATop>>::map(weak_cfg));
    }

    rp<AllocFlywheel>
    AllocFlywheel::make_default_app(const FacetAppcx & appcx)
    {
        ArenaConfig store_cfg = ArenaConfig().with_name(ArenaNameStr::from_chars("store")).with_size(256 * 1024);
        ArenaConfig strong_cfg = ArenaConfig().with_name(ArenaNameStr::from_chars("strong")).with_size(4 * 1024);
        ArenaConfig weak_cfg = ArenaConfig().with_name(ArenaNameStr::from_chars("weak")).with_size(4 * 1024);

        return make_app(appcx,
                        store_cfg,
                        strong_cfg,
                        weak_cfg);
    }

    auto
    AllocFlywheel::add_strong_ref(handle_type x) -> std::pair<handle_index_type, handle_type*>
    {
        return store_.add_strong_ref(x);
    }

    void
    AllocFlywheel::pretty(PpSink & pp) const
    {
        // TODO: pretty-printing for HandleStore

        pp.pretty_struct("AllocFlywheel"
                         // , field("foo", foo_)
                         );
    }

    std::string
    AllocFlywheel::display_string() const {
        using xo::pp::tostr;

        AllocFlywheel * self = const_cast<AllocFlywheel*>(this);

        return tostr(rp<AllocFlywheel>(self));
    }
} /*namespace xo::mm*/

/* end AllocFlywheel.cpp */
