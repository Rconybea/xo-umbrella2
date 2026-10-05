/** @file tostr.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  Improved version of tostr(), relying on temporary arena
**/

#pragma once

#include "xo/indentlog2/LogStreambuf.hpp"
#include "xo/indentlog2/LogBuffer.hpp"
#include "xo/indentlog2/TempArena.hpp"
#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/pretty.hpp>
#include <xo/reflectutil/typeseq.hpp>
#include <algorithm> // for std::min
#include <string>
#include <streambuf>

namespace xo::pp {

    /** Render @p args (concatenated, no separator) to a std::string,
     *  This implementation relies on a thread-local temporary arena
     *  for scratch space.
     *
     *  @retval string. String is heap-allocated in the ordinary way.
     **/
    template <typename... Ts>
    std::string
    tostr(const Ts &... args) {
        using xo::mm::TempArena;
        using xo::mm::DArena;
        using xo::mm::ArenaReset;

        // TempReset reset;
        DArena & arena{TempArena::local()};
        ArenaReset reset{arena};
        LogBufferAdapter buf{arena, false /*debug_flag*/};
        LogStreambuf logbuf{&buf};
        FlatSink sink{&logbuf};

        (sink.pp(args), ...);

        auto span = buf.char_used_span();
        auto retval = std::string(span.lo(), span.hi());

        return retval;
    }

}
