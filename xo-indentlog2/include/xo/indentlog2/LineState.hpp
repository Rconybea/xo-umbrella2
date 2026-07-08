/** @file LineState.hpp
 *
 *  @author Roland Conybeare, Jun 2026
 **/

#pragma once

#include <xo/arena/span.hpp>
#include <cstddef>

namespace xo {
    /** @brief Accounting for formatting state since last \n or \r.
     **/
    class LineState {
    public:
        using Span = xo::mm::span<char>;
        using ConstSpan = xo::mm::span<const char>;
        using size_t = std::size_t;
        using ptrdiff_t = std::ptrdiff_t;

    public:
        LineState() = default;
        LineState(size_t local_ppos, size_t solpos,
                  size_t color_escape_chars,
                  ptrdiff_t color_escape_start);

        size_t local_ppos() const { return local_ppos_; }
        size_t solpos() const { return solpos_; }
        size_t color_escape_chars() const { return color_escape_chars_; }
        size_t color_escape_start() const { return color_escape_start_; }

        size_t lpos() const { return local_ppos_ - solpos_; }

        /** given buffered text in [p0, pn), update line accountant state.
         *  in streambuf terminology: p0 is pbase(), pn is pptr()
         **/
        void _check_update_local_state(const char * p0,
                                       const char * pn,
                                       bool debug_flag);

        void clear() { *this = LineState(); }

    private:
        void _update_local_state_char(const char * p0,
                                      const char * pn,
                                      bool debug_flag);

    private:
        /*
         *   pbase: start of buffered text.
         *   This will be address of buf_v_[0]
         *
         *
         *   pbase                                          pptr             epptr
         *   v                  >e1<  >e2<                     v                 v
         *   |xx\xxEEExxx\xxxxxxxEExxxxEExxxxxxxEExxx\xEExxxxxx..................|
         *                ^                 ^<------new------->
         *           solpos        local_ppos
         *
         *   solpos           : first character after newline (asof)
         *   color_escape_pos : e1+e2+.. (asof)
         *   new              : new characters not (yet) reflected
         *                      in local_ppos_, color_escape_chars_ etc.
         *   asof -> as-of last call to _check_update_local_state()
         *
         *   Legend:
         *    [\] newline
         *    [x] visible character
         *    [E] color escape chars
         *
         *
         *   after _check_update_local_state():
         *
         *
         *   pbase                                          pptr             epptr
         *   v                                        >e1<     v                 v
         *   |xx\xxEEExxx\xxxxxxxEExxxxEExxxxxxxEExxx\xEExxxxxx..................|
         *                                            ^        ^
         *                                       solpos        local_ppos
         *
         */

        /** last call to _check_update_local_state() established pptr = pbase + local_ppos
         *  with this value of local_ppos_.
         **/
        size_t local_ppos_ = 0;
        /** last call to _check_update_local_state() established this position
         *  (relative to pbase) one character after last buffered \n or \r.
         **/
        size_t solpos_ = 0;
        /** number of non-printing chars after @ref solpos_
         *  comprising completed color escape sequences
         *  (ansii color escape = text delimited by '\033' and 'm')
         *  see xo/indentlog/print/color.hpp
         **/
        size_t color_escape_chars_ = 0;
        /** if >= 0:
         *  last call to _check_update_local_state() established this position
         *  (relative to pbase) for an /incomplete/ color escape sequence.
         **/
        ptrdiff_t color_escape_start_ = -1;
    };

}

/* end LineState.hpp */
