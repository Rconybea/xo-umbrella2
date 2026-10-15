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
#include <xo/ppsink/PpStyle.hpp>
#include <xo/ppsink/pretty.hpp>
#include <xo/reflectutil/typeseq.hpp>
#include <algorithm> // for std::min
#include <string>
#include <streambuf>

namespace xo::pp {

    namespace detail {
        /** tostr()'s body, through a FlatSink with style @p style **/
        template <typename... Ts>
        std::string
        tostr_styled(const PpStyle & style, const Ts &... args) {
            using xo::mm::TempArena;
            using xo::mm::DArena;
            using xo::mm::ArenaReset;

            DArena & arena{TempArena::local()};
            ArenaReset reset{arena};
            LogBufferAdapter buf{arena, false /*debug_flag*/};
            LogStreambuf logbuf{&buf};
            FlatSink sink{style, &logbuf};

            (sink.pp(args), ...);

            auto span = buf.char_used_span();
            auto retval = std::string(span.lo(), span.hi());

            return retval;
        }
    } /*namespace detail*/

    /** Render @p args (concatenated, no separator) to a std::string,
     *  This implementation relies on a thread-local temporary arena
     *  for scratch space.
     *
     *  @retval string. String is heap-allocated in the ordinary way.
     **/
    template <typename... Ts>
    std::string
    tostr(const Ts &... args) {
        return detail::tostr_styled(PpStyle::default_style(), args...);
    }

    /** As tostr(), but through a json-styled sink (PpStyle::json()): escapes
     *  in JSON's vocabulary (@c \u00NN, never @c \xNN), and no color.  So
     *  @c tojsonstr(quot(s)) is @p s as a JSON string literal.
     *  See .xo-backlog/xo-printjson/issues/04
     **/
    template <typename... Ts>
    std::string
    tojsonstr(const Ts &... args) {
        return detail::tostr_styled(PpStyle::json(), args...);
    }
}
