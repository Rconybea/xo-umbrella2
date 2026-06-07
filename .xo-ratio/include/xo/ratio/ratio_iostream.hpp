/** @file ratio_iostream.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "ratio.hpp"
#include <ostream>

namespace xo {
    namespace ratio {
        /** @brief print ratio x on stream os.
         *
         *  Example:
         *  @code
         *  print_ratio(std::cerr, make_ratio(1,2); // outputs "<ratio 1/2>"
         *  @endcode
         **/
        template <typename Int>
        void
        print_ratio (std::ostream & os, const ratio<Int> & x) {
            os << "<ratio " << x.num() << "/" << x.den() << ">";
        }

        /** @brief print ratio x on stream os.
         *
         *  Example:
         *  @code
         *  std::cout << make_ratio(2,3); // outputs "<ratio 2/3>"
         *  @endcode
         **/
        template <typename Int>
        inline std::ostream &
        operator<< (std::ostream & os, const ratio<Int> & x) {
            print_ratio(os, x);
            return os;
        }
    }
}

/** end ratio_iostream.hpp **/
