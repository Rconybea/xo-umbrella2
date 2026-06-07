/** @file backtrace.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include "backtrace.hpp"
#include <iostream>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <libunwind.h>
#include <cxxabi.h>
#include <unistd.h>
#ifndef __APPLE__
#  include <elfutils/libdwfl.h>
#endif

namespace xo {
    void
    print_backtrace(bool demangle_flag) {
        unw_cursor_t cursor;
        unw_context_t cx;

        // capture cpu register state at this call site
        unw_getcontext(&cx);

        // stack frame iterator for current thread.
        // local -> this process
        //
        unw_init_local(&cursor, &cx);

        // depth relative to top of call stack
        int depth = 0;
        while (unw_step(&cursor) > 0) {
            unw_word_t pc = 0;

            // read return address of current frame into pc.
            // This determines the function that is executing
            // when print_backtrace() invoked
            //
            unw_get_reg(&cursor, UNW_REG_IP, &pc);

            std::array<char, 256> name;
            unw_word_t offset = 0;

            // mangled function name for current frame's pc.
            //
            if (unw_get_proc_name(&cursor, name.data(), sizeof(name), &offset) == 0) {
                int status = 0;

                // we are resaponsible for calling ::free() on non-null demangled value
                char * demangled = nullptr;

                if (demangle_flag)
                    demangled = abi::__cxa_demangle(name.data(), nullptr, nullptr, &status);

                if ((status == 0) && demangled) {
                    fprintf(stderr, "#%d  0x%lx  %s+0x%lx\n",
                            depth, (long)pc, demangled, (long)offset);
                    free(demangled);
                } else {
                    // demangle failed (or disabled)
                    fprintf(stderr, "#%d  0x%lx  %s+0x%lx\n",
                            depth, (long)pc, name.data(), (long)offset);
                }
            } else {
                // unable to get function name
                fprintf(stderr, "#%d  0x%lx  ???\n", depth, (long)pc);
            }
        }
    }
    namespace {
#ifndef __APPLE__
        // libdwfl requires callbacks for find_elf and find_debuginfo.
        // The offline defaults work for the current process.
        //
        static const Dwfl_Callbacks dwfl_callbacks = {
            .find_elf = dwfl_linux_proc_find_elf,
            .find_debuginfo = dwfl_standard_find_debuginfo,
            .section_address = nullptr,
            .debuginfo_path = nullptr,
        };
#endif
    }

    void
    print_backtrace_dwarf(bool demangle_flag)
    {

#ifdef __APPLE__
        (void)demangle_flag;

        std::cerr << "backtrace with dwarf symbols not supported on osx" << std::endl;
#else
        unw_cursor_t cursor;
        unw_context_t cx;

        unw_getcontext(&cx);
        unw_init_local(&cursor, &cx);

        // set up dwfl for resolving addresses to source locations.
        //
        Dwfl * dwfl = dwfl_begin(&dwfl_callbacks);

        if (dwfl) {
            // populate module list from /proc/self/maps
            dwfl_linux_proc_report(dwfl, getpid());
            dwfl_report_end(dwfl, nullptr, nullptr);
        }

        int depth = 0;
        while (unw_step(&cursor) > 0) {
            unw_word_t pc = 0;
            unw_get_reg(&cursor, UNW_REG_IP, &pc);

            std::array<char, 256> name;
            unw_word_t offset = 0;

            // resolve function name via libunwind
            //
            const char * func_name = "???";
            char * demangled = nullptr;
            int status = -1;

            if (unw_get_proc_name(&cursor, name.data(), name.size(), &offset) == 0) {
                if (demangle_flag)
                    demangled = abi::__cxa_demangle(name.data(), nullptr, nullptr, &status);

                func_name = ((status == 0) && demangled) ? demangled : name.data();
            }

            // resolve source file + line via DWARF debug info
            //
            const char * source_file = nullptr;
            int line = 0;

            if (dwfl) {
                Dwfl_Module * module = dwfl_addrmodule(dwfl, pc);

                if (module) {
                    Dwfl_Line * dwfl_line = dwfl_module_getsrc(module, pc);

                    if (dwfl_line) {
                        source_file = dwfl_lineinfo(dwfl_line, nullptr, &line,
                                                    nullptr, nullptr, nullptr);
                    }
                }
            }

            if (source_file) {
                fprintf(stderr, "#%d  0x%lx  %s+0x%lx  at %s:%d\n",
                        depth, (long)pc, func_name, (long)offset,
                        source_file, line);
            } else {
                fprintf(stderr, "#%d  0x%lx  %s+0x%lx\n",
                        depth, (long)pc, func_name, (long)offset);
            }

            if (demangled)
                free(demangled);

            ++depth;
        }

        if (dwfl)
            dwfl_end(dwfl);
#endif

    }
} /*namespace xo*/

/* end backtrace.cpp */
