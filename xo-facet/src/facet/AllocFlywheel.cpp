/** @file AllocFlywheel.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "AllocFlywheel.hpp"
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/pretty_struct.hpp>

namespace xo::mm {
    AllocFlywheel::AllocFlywheel(DArena && storage,
                                 DArenaVector<obj<ATop>> && strong,
                                 DArenaVector<obj<ATop>> && weak)
    : store_{std::move(storage), std::move(strong), std::move(weak)}
    {}

    rp<AllocFlywheel>
    AllocFlywheel::make_app(ArenaConfig & storage_cfg,
                            ArenaConfig & strong_cfg,
                            ArenaConfig & weak_cfg)
    {
        return new AllocFlywheel(DArena::map(storage_cfg),
                                 DArenaVector<obj<ATop>>::map(strong_cfg),
                                 DArenaVector<obj<ATop>>::map(weak_cfg));
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
