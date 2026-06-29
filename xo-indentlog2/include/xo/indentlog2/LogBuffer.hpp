/** @file LogBuffer.hpp
 *
 *  @author Roland Conybeare, Jun 2026
 **/

#pragma once

#include "xo/arena/DArenaVector.hpp"

namespace xo {
    using LogBuffer = xo::mm::DArenaVector<char>;
}

/* end LogBuffer.hpp */
