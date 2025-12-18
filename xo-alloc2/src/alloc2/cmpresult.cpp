/** @file cmpresult.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "cmpresult.hpp"
#include <xo/indentlog/print/tag.hpp>
#include <iostream>

namespace xo {
    namespace mm {
        const char *
        comparison2str(comparison x)
        {
            switch (x) {
            case comparison::invalid:
                break;
            case comparison::comparable:
                return "cmp";
            case comparison::incomparable:
                return "!cmp";
            }

            return "?comparison";
        }

        void
        cmpresult::display(std::ostream & os) const
        {
            os << "<cmpresult "
               << xtag("err", err_)
               << xtag("cmp", cmp_)
               << ">";
        }
    } /*namespace mm*/
} /*namespace xo*/

/* end cmpresult.cpp */
