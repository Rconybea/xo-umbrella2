/** @file print.hpp
*
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "AllocError.hpp"
#include <xo/indentlog/print/tag.hpp>
#include <iostream>

namespace xo {
    namespace mm {
        inline std::ostream &
        operator<<(std::ostream & os, const error & x) {
            os << AllocError::error_description(x);
            return os;
        }

        inline std::ostream &
        operator<<(std::ostream & os, const AllocError & x) {
            os << "<AllocError"
               << xtag("error", x.error_)
               << xtag("seq", x.error_seq_)
               << xtag("req_z", x.request_z_)
               << xtag("commit_z", x.committed_z_)
               << xtag("resv_z", x.reserved_z_)
               << ">";
            return os;
        }
    }
}

/* end print.hpp */
