/* color.hpp */

#pragma once

#include <ostream>
//#include <utility>  // for std::move
#include <cstdint>

namespace xo {
    enum color_encoding {
        CE_None,
        CE_Ansi,
        CE_Xterm,
    };

    enum color_flags {
        CF_None = 0x0,
        CF_ColorOn = 0x01,
        CF_Contents = 0x02,
        CF_ColorOff = 0x04,
        CF_All = 0x07
    };

    template <typename Contents>
    class color_impl {
    public:
        color_impl(color_flags flags, color_encoding encoding, std::uint32_t color, Contents && contents)
            : flags_{flags}, encoding_{encoding}, color_{color}, contents_{std::forward<Contents>(contents)} {}

        std::uint32_t color() const { return color_; }
        Contents const & contents() const { return contents_; }

        void print(std::ostream & os) const {
            if ((flags_ & CF_ColorOn) && (color_ > 0)) {
                switch(encoding_) {
                case CE_None:
                    break;
                case CE_Ansi:
                    os << "\033[" << color_ << "m";
                    break;
                case CE_Xterm:
                    os << "\033[38;5;" << color_ << "m";
                    break;
                }
            }

            if (flags_ & CF_Contents)
                os << contents_;

            if ((flags_ & CF_ColorOff) && (color_ > 0)) {
                switch(encoding_) {
                case CE_None:
                    break;
                case CE_Ansi:
                case CE_Xterm:
                    os << "\033[0m";
                    break;
                }
            }
        } /*print*/

    private:
        color_flags flags_ = CF_None;

        color_encoding encoding_ = CE_Ansi;
        /* .encoding = CE_Ansi:
         *   0 = no color
         *  30 = black
         *  31 = red
         *  32 = green
         *  33 = yellow
         *  34 = blue
         *  35 = magenta
         *  36 = cyan
         *
         * .encoding = CE_Xterm:
         *   see [[https://i.stack.imgur.com/KTSQa.png]]
         *  0..7               standard colors (muted: grey, red, green, yellow, blue, pink, cyan, white)
         *  8..15              high-intensity colors (grey, red, green, yellow, blue, pink, cyan, white)
         *  16..51             chooses hue
         *  16..51 + (0..5)x36 increases whiteness
         */
        std::uint32_t color_ = 0;

        Contents contents_;
    }; /*color_impl*/

    template <typename Contents>
    color_impl<Contents> with_ansi_color(std::uint32_t color, Contents && contents) {
        return color_impl<Contents>(CF_All, CE_Ansi, color, std::forward(contents));
    } /*with_ansi_color*/

    template <typename Contents>
    color_impl<Contents> with_xterm_color(std::uint32_t color, Contents && contents) {
        return color_impl<Contents>(CF_All, CE_Xterm, color, std::forward(contents));
    } /*with_ansi_color*/

    template <typename Contents>
    color_impl<Contents> with_color(color_encoding encoding, std::uint32_t color, Contents && contents) {
        return color_impl<Contents>(CF_All, encoding, color, std::forward<Contents>(contents));
    } /*with_color*/

    inline color_impl<int>
    color_on_ansi(std::uint32_t color) {
        return color_impl<int>(CF_ColorOn, CE_Ansi, color, 0);
    } /*color_on_ansi*/

    inline color_impl<int>
    color_on_xterm(std::uint32_t color) {
        return color_impl<int>(CF_ColorOn, CE_Xterm, color, 0);
    } /*color_on_xterm*/

    inline color_impl<int>
    color_on(color_encoding encoding, std::uint32_t color) {
        return color_impl<int>(CF_ColorOn, encoding, color, 0);
    } /*color_on*/

    inline color_impl<int>
    color_off() {
        /* any non-zero value works here for color */
        return color_impl<int>(CF_ColorOff, CE_None, 1 /*color*/, 0);
    } /*color_off*/

    template <typename Contents>
    inline std::ostream &
    operator<<(std::ostream & os, color_impl<Contents> const & x) {
        x.print(os);
        return os;
    } /*operator<<*/

} /*namespace xo*/

/* end color.hpp */
