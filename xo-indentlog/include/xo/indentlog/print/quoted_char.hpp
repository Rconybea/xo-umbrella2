/* @file quoted_char.hpp */

#pragma once

#include <iostream>

namespace xo {
    template<typename CharT>
    class quoted_char {
    public:
        quoted_char(CharT ch) : ch_{ch} {}

        void print(std::ostream & os) const {
            switch(ch_) {
            case '\033':
                os << "\\033";
                break;
            case '\n':
                os << "\\n";
                break;
            case '\r':
                os << "\\r";
                break;
            default:
                os << ch_;
            }
        }

    private:
        CharT ch_;
    }; /*quoted_char*/

    template<typename CharT>
    inline std::ostream &
    operator<<(std::ostream & os, quoted_char<CharT> const & x) {
        x.print(os);
        return os;
    } /*operator<<*/

} /*namespace xo*/


/* end quoted_char.hpp */
