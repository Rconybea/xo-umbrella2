/* @file function.hpp */

#include "color.hpp"

#include <iostream>
#include <cstdint>

namespace xo {
    enum function_style {
        /* literal: print given name,  no alterations */
        FS_Literal,
        /* pretty: print name, surrounded by [] */
        FS_Pretty,
        /* streamlined: remove extraneous detail,  try to print something like class::method */
        FS_Streamlined,
        /* simple: remove everything except function/method name */
        FS_Simple
    };

    /* Tag to drive header-only expression */
    template <typename Tag>
    class function_name_impl {
    public:
        /* color: ANSI escape color (lookup Select Graphic Rendition subset)
         *        0 = none
         *        31 = red
         */
        function_name_impl(function_style style,
                           color_encoding encoding,
                           std::uint32_t color,
                           std::string_view pretty)
            : style_{style}, encoding_{encoding}, color_{color}, pretty_{pretty} {}

        function_style style() const { return style_; }
        color_encoding encoding() const { return encoding_; }
        std::uint32_t color() const { return color_; }
        std::string_view const & pretty() const { return pretty_; }

        /* e.g.
         *   std::vector<std::pair<int, xo::bar> xo::sometemplateclass<T,U>::fib(int, char**)
         *                                       ^                           ^
         *                                       p                           q
         */
        static void print_simple(std::ostream & os, std::string_view const & s) {
            std::size_t p = exclude_return_type(s);
            std::string_view s2 = s.substr(p);
            std::size_t q = find_toplevel_sep(s2, true /*last_flag*/);

            print_aux(os, s2.substr(q));
        } /*print_simple*/

        /* e.g.
         *   std::vector<std::pair<int, xo::bar> xo::sometemplateclass<T,U>::fib(int, char**)
         *                                       ^   ^
         *                                       p   q
         */
        static void print_streamlined(std::ostream & os, std::string_view const & s) {
            std::size_t p = exclude_return_type(s);
            std::string_view s2 = s.substr(p);
            std::size_t q = find_toplevel_sep(s2, false /*!last_flag*/);

            print_aux(os, s2.substr(q));
        } /*print_streamlined*/

    private:
        static std::size_t exclude_return_type(std::string_view const & s) {
            /* strategy:
             * - scan right-to-left
             * - ignore anything between matching <>, () pairs (i.e. anything nested)
             * - stop at rightmost toplevel space --> return suffix following that space
             */
            std::size_t nesting_level = 0;

            std::size_t z = s.size();
            for (std::size_t rp = 0; rp < z; ++rp) {
                std::size_t p = z-1-rp;
                char ch = s[p];

                if (ch == '<' || ch == '(')
                    ++nesting_level;

                if (nesting_level == 0)  {
                    if (ch == ' ')
                        return p + 1;
                }

                if (ch == '>' || ch == ')')
                    --nesting_level;
            }

            return 0;
        } /*exclude_return_type*/

        /* e.g.
         *    xo::ns::someclass<xo::foo, xo::bar>::somemethod(xo::enum1, std::vector<xo::blah>)
         *            ^
         *            return this pos
         *
         * last_flag:  return pos after last ::
         * !last_flag: return pos after 2nd-last ::
         */
        static std::size_t find_toplevel_sep(std::string_view const & s, bool last_flag) {
            /* strategy:
             * - scan left-to-right
             * - ignore anything between matching <>, () pairs (i.e. anything nested)
             * - count :: pairs
             * - remember 2nd-last :: pair;  reports pos just after it
             *
             * note:
             * - if no :: pairs,  or only one such pair,  return 0
             */
            std::size_t nesting_level = 0;

            std::size_t pos_after_last_sep = 0;
            std::size_t pos_after_2ndlast_sep = 0;

            for (std::size_t p = 0; p < s.size(); ++p) {
                char ch = s[p];

                if (ch == '<' || ch == '(')
                    ++nesting_level;

                if (nesting_level == 0) {
                    if ((ch == ':')
                        && (p+1 < s.size())
                        && s[p+1] == ':')
                    {
                        pos_after_2ndlast_sep = pos_after_last_sep;
                        pos_after_last_sep = p+2;
                        ++p; /* skipping 1st : in separator */
                    }
                }

                if (ch == '>' || ch == ')')
                    --nesting_level;
            }

            return last_flag ? pos_after_last_sep : pos_after_2ndlast_sep;
        } /*find_toplevel_sep*/

        /* fib(int, char **) --> fib
         * quux(std::vector<std::size_t, std::allocator<std::size_t>>) -> quux
         * foo::bar<std::vector<char>>() -> foo::bar
         */
        static void print_aux(std::ostream & os, std::string_view const & s) {
            //std::cerr << "print_aux: s=" << s << std::endl;

            /* strategy:
             * - print left-to-right,   omit anything between matching <> or () pairs.
             * - don't keep track of which is which,  so would also match < with ) etc;
             *   this acceptable since pretty functions won't visit this corner case
             */
            std::size_t nesting_level = 0;

            for (char ch : s) {
                if (ch == '<' || ch == '(')
                    ++nesting_level;

                if (nesting_level == 0)
                    os << ch;

                if (ch == '>' || ch == ')')
                    --nesting_level;
            }

        } /*print_aux*/

    private:
        /* FS_Simple | FS_Pretty (= FS_Literal) | FS_Streamlined */
        function_style style_;
        /* CE_Ansi | CE_Xterm */
        color_encoding encoding_;
        /* color,  if non-zero */
        std::uint32_t color_;
        /* e.g. __PRETTY_FUNCTION__ */
        std::string_view pretty_;
    }; /*function_name_impl*/

    using function_name = function_name_impl<class function_name_impl_tag>;

    inline std::ostream &
    operator<<(std::ostream & os,
               function_name const & fn)
    {
        /* set text color */

        switch(fn.style()) {
        case FS_Literal:
            os << with_color(fn.encoding(), fn.color(), fn.pretty());
            break;
        case FS_Pretty:
            os << "[" << with_color(fn.encoding(), fn.color(), fn.pretty()) << "]";
            break;
        case FS_Simple:
            os << color_on(fn.encoding(), fn.color());
            function_name::print_simple(os, fn.pretty());
            os << color_off();
            break;
        case FS_Streamlined:
            /* omit namespace qualifiers and template arguments */
            os << color_on(fn.encoding(), fn.color());
            function_name::print_streamlined(os, fn.pretty());
            os << color_off();
            break;
        }

        return os;
    } /*operator<<*/
} /*namespace xo*/

/* end function.hpp */
