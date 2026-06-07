/* @file filename.hpp */

#pragma once

#include <iostream>
#include <cstdint>

namespace xo {
    /* Example:
     *   os << basename("/path/to/basename.cpp")
     * prints
     *   basename.cpp
     * on os
     */

    /* Tag to drive header-only expression */
    template <typename Tag>
    class basename_impl {
    public:
        basename_impl(std::string_view path)
            : path_{path} {}

        std::string_view const & path() const { return path_; }

        /*  /home/roland/proj/nestlog/include/nestlog/filename.hpp
         *                                            <-basename->
         */
        static void print_basename(std::ostream & os, std::string_view const & s) {
            std::size_t p = exclude_dirname(s);

            os << s.substr(p);
        } /*print_basename*/

    private:
        static std::size_t exclude_dirname(std::string_view const & s) {
            std::size_t z = s.size();

            if (z == 0)
                return 0;

            if (s[z-1] == '/') {
                /* ignore trailing '/' */
                return exclude_dirname(s.substr(0, z-1));
            }

            std::size_t p = s.find_last_of('/');

            if (p == std::string_view::npos)
                return 0;
            else
                return p + 1;
        } /*exclude_dirname*/

    private:
        /* some unix pathname,  e.g. [/home/roland/proj/nestlog/include/nestlog/filename.hpp] */
        std::string_view path_;
    }; /*basename_impl*/

    using basename = basename_impl<class basename_impl_tag>;

    inline std::ostream &
    operator<<(std::ostream & os,
               basename const & bn)
    {
        basename::print_basename(os, bn.path());
        return os;
    }
} /*xo*/

/* end filename.hpp */
