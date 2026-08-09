/** @file FlatSink.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/escape.hpp>

namespace xo::pp {
    FlatSink::FlatSink(const PpStyle & style, std::ostream & os)
            : PpSink(style), os_{os}
    {}

    PpSink &
    FlatSink::put(std::string_view x)
    {
        os_.write(x.data(), x.size());
        return *this;
    }

    PpSink &
    FlatSink::put_with_escape(std::string_view x, bool quote_flag)
    {
        /* Flat output has no tokens, so there's nothing to size up front and
         * nothing that must stay contiguous: expand through a stack buffer,
         * flushing whenever the next expansion might not fit.
         *
         * Goes through Escape::str_copy() (rather than open-coding the rules
         * here) so FlatSink and PpState can't drift apart.
         */
        char buf[256];
        char * p = buf;
        /* The flush check runs before each character, so p can advance one
         * full expansion past flush_limit; reserve that, plus one byte so the
         * closing quote always fits without another flush.
         */
        char * const flush_limit = (buf + sizeof(buf)
                                    - Escape::c_max_char_expand
                                    - 1);

        if (quote_flag)
            *p++ = Escape::c_quote;

        for (char ch : x) {
            if (p > flush_limit) {
                os_.write(buf, p - buf);
                p = buf;
            }

            p = Escape::str_copy(std::string_view(&ch, 1), p);
        }

        if (quote_flag)
            *p++ = Escape::c_quote;

        if (p > buf)
            os_.write(buf, p - buf);

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
