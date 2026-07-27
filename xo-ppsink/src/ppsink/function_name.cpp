/** @file function_name.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  __PRETTY_FUNCTION__ styling, ported from legacy xo-indentlog function.hpp.
 **/

#include <xo/ppsink/function_name.hpp>
#include <string>
#include <cstdint>

namespace xo::pp {
    namespace {
        /* drop the trailing template footnote, e.g.
         *   " [with T = int]"  (gcc <13.2)  or  " [T = int]"  (clang / gcc >=13.2)
         */
        std::string_view
        exclude_template_footnote_suffix(std::string_view s) {
#if defined(__clang__)
            std::size_t p = s.find(" [");
#elif (__GNUC__ > 13) || ((__GNUC__ == 13) && (__GNUC_MINOR__ >= 2))
            std::size_t p = s.find(" [");
#else
            std::size_t p = s.find(" [with ");
#endif
            return s.substr(0, p);
        }

        /* drop a trailing " const" (const member function) */
        std::string_view
        exclude_const_suffix(std::string_view s) {
            constexpr std::size_t c_suffix_z = 6; /* strlen(" const") */

            if ((s.size() > c_suffix_z)
                && (s.substr(s.size() - c_suffix_z) == " const"))
            {
                return s.substr(0, s.size() - c_suffix_z);
            }

            return s;
        }

        /* index just past the return type: rightmost top-level space
         * (ignoring anything nested in <> or ()).
         */
        std::size_t
        exclude_return_type(std::string_view s) {
            std::size_t nesting = 0;
            std::size_t z = s.size();

            for (std::size_t rp = 0; rp < z; ++rp) {
                std::size_t p = z - 1 - rp;
                char ch = s[p];

                if (ch == '<' || ch == '(')
                    ++nesting;

                if (nesting == 0 && ch == ' ')
                    return p + 1;

                if (ch == '>' || ch == ')')
                    --nesting;
            }

            return 0;
        }

        /* index just past the last (last_flag) or 2nd-last top-level "::"
         * (ignoring nested <> / ()); 0 if fewer than that many separators.
         */
        std::size_t
        find_toplevel_sep(std::string_view s, bool last_flag) {
            std::size_t nesting = 0;
            std::size_t after_last = 0;
            std::size_t after_2ndlast = 0;

            for (std::size_t p = 0; p < s.size(); ++p) {
                char ch = s[p];

                if (ch == '<' || ch == '(')
                    ++nesting;

                if ((nesting == 0)
                    && (ch == ':') && (p + 1 < s.size()) && (s[p + 1] == ':'))
                {
                    after_2ndlast = after_last;
                    after_last = p + 2;
                    ++p; /* skip 2nd ':' of the separator */
                }

                if (ch == '>' || ch == ')')
                    --nesting;
            }

            return last_flag ? after_last : after_2ndlast;
        }

        /* append @p s to @p out, omitting anything nested in <> / () -- so
         *   fib(int, char**)                 -> fib
         *   foo::bar<std::vector<char>>()    -> foo::bar
         * with a special case so operator()'s trailing () is kept.
         */
        void
        print_aux(std::string & out, std::string_view s) {
            std::size_t nesting = 0;
            std::int32_t match_ix = 0;   /* progress matching "operator(" */
            static constexpr char const * c_target = "operator(";

            for (char ch : s) {
                if ((nesting == 0) && (ch == c_target[match_ix]) && (match_ix < 9))
                    ++match_ix;
                else
                    match_ix = 0;

                if (ch == '<') {
                    ++nesting;
                } else if (ch == '(') {
                    if ((nesting == 0) && (match_ix == 9)) {
                        /* "operator(" at top level: keep this '(' */
                    } else {
                        ++nesting;
                    }
                }

                if (nesting == 0)
                    out.push_back(ch);

                if ((nesting > 0) && ((ch == '>') || (ch == ')')))
                    --nesting;
            }
        }

        std::string
        styled_name(std::string_view pretty, bool last_flag) {
            std::string_view s2 = exclude_template_footnote_suffix(pretty);
            std::string_view s3 = exclude_const_suffix(s2);
            std::size_t q = exclude_return_type(s3);
            std::string_view s4 = s3.substr(q);
            std::size_t r = find_toplevel_sep(s4, last_flag);
            std::string_view s5 = s4.substr(r);

            std::string out;
            print_aux(out, s5);
            return out;
        }
    } /*anonymous namespace*/

    void
    put_function_name(PpSink & sink, xo::FunctionStyle style, std::string_view pretty)
    {
        switch (style) {
        case xo::FunctionStyle::literal:
            sink.put(pretty);
            break;
        case xo::FunctionStyle::pretty:
            sink.put("[");
            sink.put(pretty);
            sink.put("]");
            break;
        case xo::FunctionStyle::streamlined:
            /* Class::method: strip namespaces above the class (2nd-last "::") */
            sink.put(styled_name(pretty, false /*last_flag*/));
            break;
        case xo::FunctionStyle::simple:
            /* method: strip everything above the method name (last "::") */
            sink.put(styled_name(pretty, true /*last_flag*/));
            break;
        }
    }
} /*namespace xo::pp*/

/* end function_name.cpp */
