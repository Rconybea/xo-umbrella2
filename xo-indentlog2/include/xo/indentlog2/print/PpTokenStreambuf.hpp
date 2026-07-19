/** @file PpTokenStreambuf.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include <xo/arena/span.hpp>
#include <streambuf>
#include <cstdint>

namespace xo::print {
    class PpState; // see print/PpState.hpp

    /** @brief Temporary streambuf to assemble a PpStringToken inplace
     **/
    class PpTokenStreambuf : public std::streambuf {
    public:
        using Span = xo::mm::span<char>;
        using streamsize = std::streamsize;
        using uint32_t = std::uint32_t;

    public:
        PpTokenStreambuf(PpState * pps, uint32_t min_z);

        void commit();

        virtual int_type overflow(int_type c) override;
        std::streamsize xsputn(const char * s, streamsize n) override;

    private:

    private:
        PpState * pps_ = nullptr;

        /** writeable span.  Same as pps_->current_open_string_->mem_span() **/
        Span write_span_;
    };
} /*namespace xo::print*/

/* end PpTokenStreambuf.hpp */
