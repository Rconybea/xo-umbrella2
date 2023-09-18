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

    /* specify a color (consistent with ANSI escape sequences - the  Select Graphics Rendition subset
     * see [[https://stackoverflow.com/questions/4842424/list-of-ansi-color-escape-sequences]]
     *
     * this provides three ways to specify foreground color:
     *
     *   | enum  | escape    | example             | description   | foreground codes |
     *   +-------+-----------+---------------------+---------------+------------------+
     *   | ansi  | \033[31   | \033[31;42m         | 4-bit colors  | 30..37, 90..97   |
     *   | xterm | \033[38;5 | \033[38;5;143m      | 8-bit colors  | 0..255           |
     *   | rgb   | \033[38;2 | \033[38;2;10;20;30m | 24-bit colors | 3x 0..255        |
     *
     */
    class color_spec {
    public:
        color_spec() = default;
        color_spec(color_encoding encoding, std::uint32_t code)
            : encoding_{encoding}, code_{code} {}

        static color_spec none() { return color_spec(); }
        static color_spec ansi(std::uint32_t code) { return color_spec(CE_Ansi, code); }
        static color_spec xterm(std::uint32_t code) { return color_spec(CE_Xterm, code); }
        static color_spec rgb(std::uint8_t red, std::uint8_t green, std::uint8_t blue) {
            return none();
            //return color_spec(CE_Rgb, (red << 16 | green << 8 | blue));
        }

        /* 4-bit foreground colors */
        static color_spec black          () { return ansi(30); }
        static color_spec red            () { return ansi(31); }
        static color_spec green          () { return ansi(32); }
        static color_spec yellow         () { return ansi(33); }
        static color_spec blue           () { return ansi(34); }
        static color_spec magenta        () { return ansi(35); }
        static color_spec cyan           () { return ansi(36); }
        static color_spec white          () { return ansi(37); }
        static color_spec bright_black   () { return ansi(90); }
        static color_spec bright_red     () { return ansi(91); }
        static color_spec bright_green   () { return ansi(92); }
        static color_spec bright_yellow  () { return ansi(99); }
        static color_spec bright_blue    () { return ansi(94); }
        static color_spec bright_magenta () { return ansi(95); }
        static color_spec bright_cyan    () { return ansi(96); }
        static color_spec bright_white   () { return ansi(97); }

        color_encoding encoding() const { return encoding_; }
        std::uint32_t code() const { return code_; }

        void print_fg_color_on (std::ostream & os) const {
            switch (encoding_) {
            case CE_None:
                break;
            case CE_Ansi:
                os << "\033[31;" << code_ << "m";
                break;
            case CE_Xterm:
                os << "\033[38;5;" << code_ << "m";
                break;
            }
        } /*print_fg_color_on*/

        /* escape to reverse effect of .print_on() */
        void print_fg_color_off (std::ostream & os) const {
            switch (encoding_) {
            case CE_None:
                break;
            case CE_Ansi:
            case CE_Xterm:
                os << "\033[0m";
                break;
            }
        } /*print_fg_color_off*/

    private:
        /* none | ansi | xterm | rgb */
        color_encoding encoding_ = CE_None;
        /* ansi  : 30..37, 90..97
         * xterm : 0..255
         *   see [[https://i.stack.imgur.com/KTSQa.png]]
         *   0..7               standard colors (muted: grey, red, green, yellow, blue, pink, cyan, white)
         *   8..15              high-intensity colors (grey, red, green, yellow, blue, pink, cyan, white)
         *   16..51             chooses hue
         *   16..51 + (0..5)x36 increases whiteness
         * rgb   : r={hi 8 bits}, g={mid 8 bits}, b={lo 8 bits}
         */
        std::uint32_t code_ = 0;
    }; /*color_spec*/

    enum color_flags {
        CF_None = 0x0,
        CF_ColorOn = 0x01,
        CF_Contents = 0x02,
        CF_ColorOff = 0x04,
        CF_All = 0x07
    };

    /* stream-insertable color control */
    template <typename Contents>
    class color_impl {
    public:
        color_impl(color_flags flags, color_spec spec, Contents && contents)
            : flags_{flags}, spec_{spec}, contents_{std::forward<Contents>(contents)} {}

        color_spec const & spec() const { return spec_; }
        std::uint32_t color() const { return spec_.code(); }
        Contents const & contents() const { return contents_; }

        void print(std::ostream & os) const {
            if (flags_ & CF_ColorOn)
                spec_.print_fg_color_on(os);

            if (flags_ & CF_Contents)
                os << contents_;

            if (flags_ & CF_ColorOff)
                spec_.print_fg_color_off(os);
        } /*print*/

    private:
        /* controls independently what to print
         *     \033[38;5;117m hello, world! \033[0m
         *     <------------> <-----------> <----->
         *       CF_ColorOn    CF_Contents   CF_ColorOff
         */
        color_flags flags_ = CF_None;

        color_spec spec_;

        Contents contents_;
    }; /*color_impl*/

    template <typename Contents>
    color_impl<Contents> with_color(color_spec spec, Contents && contents) {
        return color_impl<Contents>(CF_All, spec, std::forward<Contents>(contents));
    } /*with_color*/

    inline color_impl<int>
    color_on(color_spec spec) {
        return color_impl<int>(CF_ColorOn, spec, 0);
    } /*color_on*/

    inline color_impl<int>
    color_off() {
        /* any spec other than color_spec::none() works here */
        return color_impl<int>(CF_ColorOff, color_spec::white(), 0);
    } /*color_off*/

    template <typename Contents>
    inline std::ostream &
    operator<<(std::ostream & os, color_impl<Contents> const & x) {
        x.print(os);
        return os;
    } /*operator<<*/

} /*namespace xo*/

/* end color.hpp */
