/** @file pretty.cpp
*
 *  @author Roland Conybeare, Jul 2026
 **/

#include "pretty.hpp"

namespace xo::pp {

    template <>
    void dwim(PpSink & sink, const begin & x)
    {
        sink.begin(x.offset_);
    }

    template <>
    void dwim(PpSink & sink, const split & x)
    {
        sink.split(x.spaces_, x.offset_);
    }

    template <>
    void dwim(PpSink & sink, const newline & x)
    {
        sink.newline(x.offset_);
    }

    template <>
    void dwim(PpSink & sink, const detail::_end &)
    {
        sink.end();
    }

} /*namespace xo::pp*/

/* end pretty.cpp */
