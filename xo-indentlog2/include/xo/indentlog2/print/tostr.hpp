/** @file tostr.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  Improved version of tostr(), relying on temporary arena
**/

#pragma once

#include <xo/arena/TempArena.hpp>
#include <xo/ppsink/FlatSink.hpp>
#include <xo/reflectutil/typeseq.hpp>
#include <algorithm> // for std::min
#include <string>
#include <streambuf>

namespace xo::pp {

    struct fixed_streambuf : std::streambuf {
        fixed_streambuf(char* p, std::size_t n) {
            setp(p, p + n);        // put area: [p, p+n)
            setg(p, p, p + n);     // get area, if you also read
        }
        // Non-expandable: signal failure instead of growing.
        int_type overflow(int_type) override { return traits_type::eof(); }

        // Expose what was written.
        std::string str() const {
            return std::string(pbase(), pptr());   // [start, current)
        }
        std::size_t size() const { return pptr() - pbase(); }
    };

    /** Render @p args (concatenated, no separator) to a std::string,
     *  This implementation relies on a thread-local temporary arena
     *  for scratch space.
     *
     *  @retval string. String is heap-allocated in the ordinary way.
     **/
    template <typename... Ts>
    std::string
    tostr(const Ts &... args) {
        // TempReset reset;
        DArena & arena{TempArena::local()};
        ArenaReset reset{arena};
        LogBufferAdapter buf{arena};
        LogStreambuf sbuf{&buf};
        ostream ss{&sbuf};
        FlatSink sink{ss};

        (sink.pp(args), ...);

        auto retval = sbuf.str();

        return retval;
    }

}
