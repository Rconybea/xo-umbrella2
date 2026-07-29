/** @file print.hpp
*
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "AllocError.hpp"
#include <xo/ppsink/tag_ostream.hpp>
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
            using xo::pp::xtag;

            os << "<AllocError"
               << xtag("error", x.error_);

            if (x.src_fn_)
                os << xtag("src_fn", x.src_fn_);

            os << xtag("seq", x.error_seq_)
               << xtag("req_z", x.request_z_)
               << xtag("commit_z", x.committed_z_)
               << xtag("resv_z", x.reserved_z_)
               << ">";

            return os;
        }
    }
}

/* end print.hpp */
