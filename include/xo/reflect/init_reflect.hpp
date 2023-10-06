/* file init_reflect.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

#include "xo/subsys/Subsystem.hpp"

namespace xo {
  /* tag to represent the reflect/ subsystem within ordered initialization */
  enum S_reflect_tag {};

  template<>
  struct InitSubsys<S_reflect_tag> {
    static void init();
    static InitEvidence require();
  };
} /*namespace xo*/


/* end init_reflect.hpp */
