/* file init_filter.hpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#pragma once

#include "subsys/Subsystem.hpp"

namespace xo {
  enum S_filter_tag {};

  template<>
  struct InitSubsys<S_filter_tag> {
    static void init();
    static InitEvidence require();
  };
} /*namespace xo*/

/* end init_filter.hpp */
