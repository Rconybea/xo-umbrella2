/* file init_printjson.cpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#include "init_printjson.hpp"
#include "xo/reflect/init_reflect.hpp"
#include "xo/subsys/Subsystem.hpp"

namespace xo {
  void
  InitSubsys<S_printjson_tag>::init()
  {
    /* placeholder -- expecting there to be non-trivial content soon */
  } /*init*/

  InitEvidence
  InitSubsys<S_printjson_tag>::require()
  {
    InitEvidence retval;

    /* subsystem dependencies for printjson/ */
    retval ^= InitSubsys<S_reflect_tag>::require();

    /* printjson/'s own initialization code */
    retval ^= Subsystem::provide<S_printjson_tag>("printjson", &init);

    return retval;
  } /*require*/
} /*namespace xo*/

/* end init_printjson.cpp */
