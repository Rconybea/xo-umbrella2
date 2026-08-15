/** @file TempArena.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#include "TempArena.hpp"

namespace xo::mm {
    uint32_t
    TempArena::s_cap = 4 * 1024;

    void
    TempArena::init(uint32_t cap)
    {
        s_cap = cap;
    }

    DArena &
    TempArena::local()
    {
        /** allocate temp arenas per thread **/
        static thread_local DArena * s_local = nullptr;

        if (!s_local) {
            ArenaConfig cfg
                = (ArenaConfig()
                   .with_name("scratch")
                   .with_size(s_cap));

            s_local = new DArena(cfg);
        }

        return *s_local;
    }
} /*namespace xo::mm*/

/* end TempArena.cpp */
