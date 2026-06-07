/** @file backtrace.hpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

namespace xo {
    void print_backtrace(bool demangle_flag);
    void print_backtrace_dwarf(bool demangle_flag);
}

/* end backtrace.hpp */
