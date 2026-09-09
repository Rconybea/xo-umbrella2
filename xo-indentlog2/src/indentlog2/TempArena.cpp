/** @file TempArena.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#include "TempArena.hpp"

namespace xo::mm {
    uint32_t
    TempArena::s_cap = 4 * 1024;

    thread_local std::unique_ptr<DArena>
    TempArena::s_local;

    void
    TempArena::init(uint32_t cap)
    {
        s_cap = cap;
    }

    DArena *
    TempArena::check_local()
    {
        return s_local.get();
    }

    DArena &
    TempArena::local()
    {
        if (!s_local) {
            ArenaConfig cfg
                = (ArenaConfig()
                   .with_name(ArenaNameStr::from_chars("scratch"))
                   .with_size(s_cap));

            s_local.reset(new DArena(cfg));
        }

        return *s_local;
    }
} /*namespace xo::mm*/

/* end TempArena.cpp */
