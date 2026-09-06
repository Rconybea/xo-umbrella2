/** @file PpTokenStreambuf.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include <xo/arena/span.hpp>
#include <cstdint>
#include <streambuf>

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

        /** re-point this streambuf at @p pps.
         *
         *  For PrettySink's move ctor ONLY: a PpTokenStreambuf refers to a
         *  PpState that is its SIBLING member, so a memberwise move leaves
         *  this pointer aimed at the moved-from object.  See
         *  PrettySink(PrettySink&&).
         *
         *  Does not disturb the put area: those pointers address arena memory,
         *  which a DArena move transfers without relocating.
         **/
        void reset_pps(PpState * pps) noexcept { pps_ = pps; }

        /** the PpState this streambuf writes into.  Lets PrettySink assert the
         *  sbuf_ -> pps_ link survived a move -- see PrettySink::verify_ok
         **/
        const PpState * _pps() const noexcept { return pps_; }

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
