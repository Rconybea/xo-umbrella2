/** @file FlatSink.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include <xo/ppsink/FlatSink.hpp>

namespace xo::pp {
    PpSink &
    FlatSink::put(std::string_view x)
    {
        os_.write(x.data(), x.size());
        return *this;
    }

    PpSink &
    FlatSink::begin()
    {
        /* flat output discards group structure */
        return *this;
    }

    PpSink &
    FlatSink::begin(std::int32_t /*offset*/)
    {
        /* flat output discards group structure (and its indent) */
        return *this;
    }

    PpSink &
    FlatSink::split(std::uint32_t spaces, std::int32_t /*offset*/)
    {
        /* flat output never breaks: render a split as its flat spaces */
        for (std::uint32_t i = 0; i < spaces; ++i)
            os_.put(' ');
        return *this;
    }

    PpSink &
    FlatSink::newline(std::int32_t /*offset*/)
    {
        /* a forced break is a hard newline even in flat output (no indent) */
        os_.put('\n');
        return *this;
    }

    PpSink &
    FlatSink::end()
    {
        /* flat output discards group structure */
        return *this;
    }

    PpSinkInserter
    FlatSink::stream_open(uint32_t /*min_z*/)
    {
        /* no token to reserve: operator<< writes straight to os_ */
        return PpSinkInserter(this, &os_);
    }

    void
    FlatSink::stream_commit()
    {
        /* nothing to finalize - no temporary stream */
    }

} /*namespace xo::pp*/

/* end FlatSink.cpp */
