/* file Reactor.cpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#include "Reactor.hpp"

namespace xo {
  namespace reactor {
    void
    Reactor::run_n(int32_t n)
    {
      if (n == -1) {
	for (;;) {
	  this->run_one();
	}
      } else {
	for (int32_t i=0; i<n; ++i) {
	  this->run_one();
	}
      }
    } /*run_n*/
  } /*namespace reactor*/
} /*namespace xo*/

/* end Reactor.cpp */
