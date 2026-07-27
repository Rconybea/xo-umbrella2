/** @file PpTokenStreambuf.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include <xo/arena/span.hpp>
#include <streambuf>
#include <cstdint>

namespace xo::pp {
    class PpState; // see print/PpState.hpp

    /** @brief Temporary streambuf to assemble a PpStringToken inplace
     **/
    class PpTokenStreambuf : public std::streambuf {
    public:
        using Span = xo::mm::span<char>;
        using streamsize = std::streamsize;
        using uint32_t = std::uint32_t;

    public:
        explicit PpTokenStreambuf(PpState * pps);

        /** open a new string at the sink end of @ref *pps_;
         *  write to that string with this streambuf.
         *
         *  Require: open() and close() calls must strictly alternate
         **/
        void open(uint32_t min_z);
        /** commit string previously created with open **/
        void commit();

        virtual int_type overflow(int_type c) override;
        std::streamsize xsputn(const char * s, streamsize n) override;

    private:

    private:
        PpState * pps_ = nullptr;

#ifdef OBSOLETE
        /** writeable span.  Same as pps_->current_open_string_->mem_span() **/
        Span write_span_;
#endif
    };
} /*namespace xo::pp*/

/* end PpTokenStreambuf.hpp */
