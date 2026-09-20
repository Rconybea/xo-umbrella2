/** @file AllocFlywheel.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "AllocFlywheel.hpp"
#include "TypeRegistry.hpp"
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/pretty_struct.hpp>

namespace xo::facet {
    using xo::mm::ArenaConfig;
    using xo::mm::MemorySizeInfo;

    AllocFlywheel::AllocFlywheel(const FacetAppcx & facet_appcx,
                                 DArena && storage,
                                 DArenaVector<ObjectSlot> && strong,
                                 DArenaVector<handle_index_type> && strong_freelist)
    : facetappcx_creation_evidence_{facet_appcx.creation_evidence()},
      indentlog2appcx_creation_evidence_{facet_appcx.indentlog2appcx_creation_evidence()},
      store_{std::move(storage),
             std::move(strong),
             std::move(strong_freelist)}
    {
        // facet_appcx: proof of work: facet,indentlog2 init performed;
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
                            const ArenaConfig & storage_cfg_in,
                            const ArenaConfig & strong_cfg)
    {
        auto strong = DArenaVector<ObjectSlot>::map(strong_cfg);
        auto strong_freelist = make_freelist(strong_cfg, strong.capacity());

        ArenaConfig storage_cfg
            = (storage_cfg_in
               .with_store_header_flag(true)
               .with_base_align_z(appcx.config().storage_base_align()));

        /* alloc headers forced on, overriding whatever the caller asked for.
         * DHandleStore requires them (see its ctor), so honouring a false here
         * would only produce a throw one layer down.
         */
        return new AllocFlywheel(appcx,
                                 DArena::map(storage_cfg),
                                 std::move(strong), std::move(strong_freelist));
    }

    rp<AllocFlywheel>
    AllocFlywheel::make_default_app(const FacetAppcx & appcx)
    {
        ArenaConfig store_cfg = ArenaConfig().with_name(ArenaNameStr::from_chars("store")).with_size(256 * 1024);
        ArenaConfig strong_cfg = ArenaConfig().with_name(ArenaNameStr::from_chars("strong")).with_size(4 * 1024);

        return make_app(appcx,
                        store_cfg,
                        strong_cfg);
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

    FlywheelInfo
    AllocFlywheel::snapshot() const
    {
        FlywheelInfo retval;

        /* pools first, in the order the store reports them, so pool_v_[0] is
         * the storage arena and the root-set arenas follow.
         *
         * MemorySizeInfo reported AS-IS, not copied into a shadow struct: it is
         * already the right shape, and reflecting it directly means there is
         * nothing to keep in step.  Its detail_ is excluded by the reflection
         * (see reflect_flywheel_info), not by copying around it.
         */
        store_.visit_pools([&retval](const MemorySizeInfo & x) {
                retval.pool_v_.push_back(x);
            });

        /* the root set is BORROWED, not copied: the frame reads it when it
         * is printed, not now.  See FlywheelInfo::strong_ -- retiring
         * RootSetInfo is what removed the copy, and this is what it costs.
         */
        retval.strong_ = &store_;

        return retval;
    }

    auto
    AllocFlywheel::strong_root_count() const -> handle_index_type
    {
        return store_.strong_root_count();
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
} /*namespace xo::facet*/

/* end AllocFlywheel.cpp */
