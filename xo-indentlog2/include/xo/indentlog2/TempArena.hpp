/** @file TempArena.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#pragma once

#include <xo/arena/DArena.hpp>
#include <cstdint>

namespace xo::mm {
    /** Per-thread scratch space
     *  Use:
     *    void foo() {
     *      // alloc a Bar instance from thread-local arena
     *      auto m = TempArena::alloc<Bar>(...);
     *      m.size(); // sizeof(Bar)
     *      m.data(); // &Bar
     *
     *      // memory recovered when m goes out of scope
     *    }
     **/
    class TempArena {
    public:
        using uint32_t = std::uint32_t;

    public:
        TempArena() = default;

        /** Initialize (just remembers @p cap).
         **/
        static void init(uint32_t cap);

        /** Thread-local temporary space.
         *  Allocated on demand, once per thread.
         **/
        static DArena & local();

    private:
        /** capacity for thread-local arenas **/
        static uint32_t s_cap;
    };
}

/* end TempArena.hpp */
