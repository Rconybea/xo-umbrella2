/* file AbstractRealization.hpp
 *
 * author: Roland Conybeare, Nov 2022
 */

#pragma once

#include "xo/reflect/SelfTagging.hpp"
#include "AbstractStochasticProcess.hpp"

namespace xo {
  namespace process {
    class AbstractRealization : public reflect::SelfTagging {
    public:
      virtual rp<AbstractStochasticProcess> stochastic_process() const = 0;
    }; /*AbstractRealization*/
  } /*namespace process*/
} /*namespace xo*/

/* end AbstractRealization.hpp */
