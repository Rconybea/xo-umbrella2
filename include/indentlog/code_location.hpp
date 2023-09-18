/* @file code_location.hpp */

#pragma once

#include "filename.hpp"
#include "color.hpp"

namespace xo {
    /* Example:
     *   os << code_location("/path/to/foo.cpp", 123)
     * writes
     *   foo.cpp:123
     * on stream os
     */

    /* Tag to drive header-only expression */
    template <typename Tag>
    class code_location_impl {
    public:
        code_location_impl(std::string_view file,
                           std::uint32_t line,
                           color_encoding encoding = CE_Ansi,
                           std::uint32_t color = 31 /*red*/)
            : file_{file}, line_{line}, encoding_{encoding}, color_{color} {}

        void print_code_location(std::ostream & os) const {
            os << "["
               << with_color(color_spec(encoding_, color_), basename(file_))
               << ":"
               << line_
               << "]";
        } /*print_code_location*/

    private:
        /* __FILE__ */
        std::string_view file_;
        /* __LINE__ */
        std::uint32_t line_ = 0;
        /* color encoding for file,line */
        color_encoding encoding_ = CE_Ansi;
        /* color for file,line */
        std::uint32_t color_ = 0;
    }; /*code_location_impl*/

    using code_location = code_location_impl<class code_location_impl_tag>;

    inline std::ostream &
    operator<<(std::ostream & os,
               code_location const & x)
    {
        x.print_code_location(os);
        return os;
    }
} /*namespace xo*/

/* end code_location.hpp */
