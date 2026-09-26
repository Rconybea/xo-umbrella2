/** @file ArenaConfig.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "ArenaConfig.hpp"
#include <xo/ppsink/pretty_struct.hpp>

namespace xo {
    using xo::pp::PpSink;
    using xo::pp::field;

    namespace mm {

        ArenaConfig
        ArenaConfig::with_name(const ArenaNameStr & name) const
        {
            ArenaConfig copy(*this);
            copy.name_ = name;
            return copy;
        }

        ArenaConfig
        ArenaConfig::with_size(std::size_t z) const
        {
            ArenaConfig copy(*this);
            copy.size_ = z;
            return copy;
        }

        ArenaConfig
        ArenaConfig::with_base_align_z(std::size_t z) const
        {
            ArenaConfig copy(*this);
            copy.base_align_z_ = z;
            return copy;
        }

        ArenaConfig
        ArenaConfig::with_exclusive_block_flag(bool x) const
        {
            ArenaConfig copy(*this);
            copy.exclusive_block_flag_ = x;
            return copy;
        }

        ArenaConfig
        ArenaConfig::with_store_header_flag(bool x) const
        {
            ArenaConfig copy(*this);
            copy.store_header_flag_ = x;
            return copy;
        }

        void
        ArenaConfig::pretty(PpSink & sink) const
        {
            /* name omitted when empty -- it is optional, for diagnostics only.
             * header_ renders through Prettifier<AllocHeaderConfig>.
             */
            sink.pretty_struct("ArenaConfig",
                               field("name", name_, !name_.empty()),
                               field("size", size_),
                               field("hugepage_z", hugepage_z_),
                               field("store_header", store_header_flag_),
                               field("header", header_),
                               field("debug", debug_flag_, debug_flag_));
        }
    } /*namespace mm*/
} /*namespace xo*/

/* end ArenaConfig.cpp */
