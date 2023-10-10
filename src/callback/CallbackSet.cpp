/* file CallbackSet.cpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#include "CallbackSet.hpp"

namespace xo {
    namespace fn {
        CallbackId
        CallbackId::generate()
        {
            static CallbackId s_last_id;

            s_last_id = CallbackId(s_last_id.id() + 1);

            return s_last_id;
        } /*generate*/
    } /*namespace fn*/
} /*namespace xo*/

/* end CallbackSet.cpp */
