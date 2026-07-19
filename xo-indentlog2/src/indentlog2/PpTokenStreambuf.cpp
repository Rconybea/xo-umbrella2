/** @file PpTokenStreambuf.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "print/PpTokenStreambuf.hpp"
#include "print/PpState.hpp"

namespace xo::print {
    PpTokenStreambuf::PpTokenStreambuf(PpState * pps, uint32_t min_z)
      : pps_{pps}
    {
        auto span = pps_->open_string(min_z);

        this->setp(span.lo(), span.hi());
    }

    void
    PpTokenStreambuf::commit()
    {
        pps_->commit_string(Span(this->pbase(), this->pptr()));
        pps_ = nullptr;
    }

    auto
    PpTokenStreambuf::overflow(int_type c) -> int_type
    {
        if (traits_type::eq_int_type(c, traits_type::eof())) {
            // flush request. no-op for PpTokenStreambuf
            return traits_type::not_eof(c);
        }

        // 1. finalize buffered chars as a single string token
        pps_->commit_string(Span(this->pbase(), this->pptr()));

        // 2. Estasblih range big enough to hold at least one char

        auto span = pps_->open_string(1);

        // 3. connect token's char array to this streambuf

        this->setp(span.lo(), span.hi());
        *(this->pptr()) = traits_type::to_char_type(c);
        this->pbump(1);

        return c;
    }

    auto
    PpTokenStreambuf::xsputn(const char * s, streamsize n) -> streamsize
    {
        streamsize left = n;

        while (left > 0) {
            streamsize room = this->epptr() - this->pptr();

            if (room == 0) {
                // overflow: get more buffer space and/or print some formatted content.
                // on success writes exactly one char
                if (this->overflow(traits_type::to_int_type(*s)) == traits_type::eof()) {
                    // really out of room
                    break;
                }

                ++s;
                --left;
                continue;
            }

            streamsize n2copy = std::min(left, room);

            std::memcpy(this->pptr(), s, n2copy);
            this->pbump(n2copy);

            s += n2copy;
            left -= n2copy;
        }

        return n - left;
    }

}

/* end PpTokenStreambuf.cpp */
