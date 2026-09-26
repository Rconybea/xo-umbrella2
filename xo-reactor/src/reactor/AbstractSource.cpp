/* @file AbstractSource.cpp */

#include "AbstractSource.hpp"
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>

namespace xo {
    namespace reactor {

        uint64_t
        AbstractSource::deliver_n(uint64_t n)
        {
            uint64_t retval = 0;

            for (uint64_t i=0; i<n; ++i) {
                uint64_t n1 = this->deliver_one();

                if (n1 == 0) {
                    /* short-circuit if source has less than n
                     * events available
                     */
                    break;
                }

                retval += n1;
            }

            return retval;
        } /*deliver_n*/

        uint64_t
        AbstractSource::deliver_all()
        {
            uint64_t retval = 0;

            for (;;) {
                uint64_t n1 = this->deliver_one();

                if (n1 == 0)
                    break;

                retval += n1;
            }

            return retval;
        } /*deliver_all*/

    } /*namespace reactor*/
} /*namespace xo*/

/* end AbstractSource.cpp */
