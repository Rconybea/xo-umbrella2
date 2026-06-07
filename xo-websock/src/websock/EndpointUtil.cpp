/* file EndpointUtil.cpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#include "EndpointUtil.hpp"

namespace xo {
    namespace web {
        std::string
        EndpointUtil::stem(std::string const & pattern)
        {
            std::size_t p = 0;
            do {
                p = pattern.find_first_of("$", p);

                if ((p != std::string::npos) && (pattern[p+1] == '{')) {
                    /* fixed stem is chars [0 .. p-1],  i.e. 1st p characters */
                    break;
                }

                if (p != std::string::npos) {
                    /* skip to next '$' */
                    ++p;
                }
            } while (p != std::string::npos);

            if (p == std::string::npos) {
                /* pattern has no variable components */
                return pattern;
            } else {
                return pattern.substr(0, p);
            }
        } /*stem*/
    } /*namespace web*/
} /*namespace xo*/

/* end EndpointUtil.cpp */
