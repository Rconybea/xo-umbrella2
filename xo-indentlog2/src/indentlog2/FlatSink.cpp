/** @file FlatSink.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "print/FlatSink.hpp"

namespace xo::print {
    void
    FlatSink::put(std::string_view x)
    {
        os_.write(x.data(), x.size());
    }

    void
    FlatSink::begin()
    {
        /* flat output discards group structure */
    }

    void
    FlatSink::split()
    {
        /* flat output ignores splits */
    }

    void
    FlatSink::end()
    {
        /* flat output discards group structure */
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

} /*namespace xo::print*/

/* end FlatSink.cpp */
