/** @file typeseq.hpp
 *
 *  @brief Re-export typeseq from xo-arena for backwards compatibility
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <xo/reflectutil/typeseq.hpp>

namespace xo {
    namespace facet {
        // Re-export from xo::arena namespace
        using xo::reflect::typerecd;
        using xo::reflect::typeseq;
    }
} /*namespace xo*/

/* end typeseq.hpp */
