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
                                 DArenaVector<handle_index_type> && strong_freelist,
                                 DArenaVector<obj<ATop>> && weak,
                                 DArenaVector<handle_index_type> && weak_freelist)
      : facet_appcx_{facet_appcx},
        store_{std::move(storage),
               std::move(strong), std::move(strong_freelist),
               std::move(weak), std::move(weak_freelist)}
    {
        // facet_appcx_: proof of work: facet,indentlog2 init performed;
        // implies config-dependent globals setup, including:
        // - FacetRegistry
        // - TempArena
        // - TempPrettySink
        // - SinkFactory
    }

    namespace {
        /** free-list arena serving a root set of @p capacity slots **/
        DArenaVector<AllocFlywheel::handle_index_type>
        make_freelist(const ArenaConfig & root_cfg, std::size_t capacity)
        {
            using handle_index_type = AllocFlywheel::handle_index_type;

            /* name it after the set it serves, so the pool report pairs them
             * without the reader having to know the order
             */
            ArenaNameStr name = ArenaNameStr::sprintf("%s-free", root_cfg.name().c_str());

            return DArenaVector<handle_index_type>::map
                (ArenaConfig()
                     .with_name(name)
                     .with_size(capacity * sizeof(handle_index_type)));
        }
    } /*namespace*/

    rp<AllocFlywheel>
    AllocFlywheel::make_app(const FacetAppcx & appcx,
                            const ArenaConfig & storage_cfg,
                            const ArenaConfig & strong_cfg,
                            const ArenaConfig & weak_cfg)
    {
        auto strong = DArenaVector<obj<ATop>>::map(strong_cfg);
        auto weak = DArenaVector<obj<ATop>>::map(weak_cfg);

        auto strong_freelist = make_freelist(strong_cfg, strong.capacity());
        auto weak_freelist = make_freelist(weak_cfg, weak.capacity());

        return new AllocFlywheel(appcx,
                                 DArena::map(storage_cfg),
                                 std::move(strong), std::move(strong_freelist),
                                 std::move(weak), std::move(weak_freelist));
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
    AllocFlywheel::remove_strong_ref(handle_index_type ix)
    {
        store_.remove_strong_ref(ix);
    }

    void
    AllocFlywheel::remove_weak_ref(handle_index_type ix)
    {
        store_.remove_weak_ref(ix);
    }

    auto
    AllocFlywheel::strong_root_count() const -> handle_index_type
    {
        return store_.strong_root_count();
    }

    auto
    AllocFlywheel::weak_root_count() const -> handle_index_type
    {
        return store_.weak_root_count();
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
