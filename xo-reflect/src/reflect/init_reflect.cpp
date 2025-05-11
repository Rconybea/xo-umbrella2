/* file init_reflect.cpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#include "init_reflect.hpp"
#include "xo/subsys/Subsystem.hpp"

namespace xo {
  void
  InitSubsys<S_reflect_tag>::init()
  {
    /* placeholder -- expecting there to be non-trivial content soon */
  } /*init*/

  InitEvidence
  InitSubsys<S_reflect_tag>::require()
  {
    return Subsystem::provide<S_reflect_tag>("reflect", &init);
  } /*require*/
} /*namespace xo*/

/* end init_reflect.cpp */
