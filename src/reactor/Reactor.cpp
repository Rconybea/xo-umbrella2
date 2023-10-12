/* file Reactor.cpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#include "Reactor.hpp"
#include "init_reactor.hpp"
#include "xo/subsys/Subsystem.hpp"

namespace xo {
    namespace reactor {
        Reactor::Reactor() {
            /* ensure reactor subsystem + deps initialized */

            InitSubsys<S_reactor_tag>::require();

            Subsystem::initialize_all();
        }

        std::uint64_t
        Reactor::run_n(int32_t n)
        {
            std::uint64_t retval = 0;

            if (n == -1) {
                for (;;) {
                    retval += this->run_one();
                }
            } else {
                for (int32_t i=0; i<n; ++i) {
                    retval += this->run_one();
                }
            }

            return retval;
        } /*run_n*/
    } /*namespace reactor*/
} /*namespace xo*/

/* end Reactor.cpp */
