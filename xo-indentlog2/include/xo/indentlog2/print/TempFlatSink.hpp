/** @file TempFlatSink.hpp
 *
 *  @author Roland Conybeare, Oct 2026
 *
 *  TempFlatSink: a FlatSink over per-thread scratch memory (TempArena),
 *  whose text is read back with output().
 **/

#pragma once

#include "xo/indentlog2/LogStreambuf.hpp"
#include "xo/indentlog2/LogBuffer.hpp"
#include "xo/indentlog2/TempArena.hpp"
#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <string_view>

namespace xo::pp {
    /** @brief a FlatSink with style @p style, writing into scratch memory.
     *
     *  For rendering to text without a heap stream -- e.g. json:
     *  @code
     *    TempFlatSink out(PpStyle::json());
     *    pjson.print_tp(tp, &out.sink());
     *    std::string_view text = out.output();
     *  @endcode
     *
     *  Memory comes from TempArena::local(), and goes back to it when this
     *  does (ArenaReset).  So temp sinks, and tostr() which is built on
     *  one, nest LIFO: one made and destroyed while another is alive is
     *  fine; destroying them out of order is not.
     *
     *  Not copyable or movable: the sink and its buffers point at each
     *  other.
     **/
    class TempFlatSink {
    public:
        explicit TempFlatSink(const PpStyle & style)
            : sink_{style, &logbuf_} {}

        TempFlatSink(const TempFlatSink &) = delete;
        TempFlatSink & operator=(const TempFlatSink &) = delete;

        /** where to write **/
        FlatSink & sink() { return sink_; }

        /** everything written so far.  Valid until the next write, or
         *  until this is destroyed
         **/
        std::string_view output() const {
            auto span = buf_.char_used_span();

            return std::string_view(span.lo(), span.hi() - span.lo());
        }

    private:
        /* member order is construction order -- and reverse destruction
         * order: the arena is restored after everything using it is gone
         */

        /** returns scratch memory to TempArena on destruction **/
        xo::mm::ArenaReset reset_{xo::mm::TempArena::local()};
        /** the text, in scratch memory **/
        LogBufferAdapter buf_{xo::mm::TempArena::local(), false /*debug_flag*/};
        /** streambuf over buf_ **/
        LogStreambuf logbuf_{&buf_};
        /** the sink **/
        FlatSink sink_;
    };
} /*namespace xo::pp*/

/* end TempFlatSink.hpp */
