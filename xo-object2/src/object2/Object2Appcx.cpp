/** @file Object2Appcx.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "cx/Object2Appcx.hpp"
#include "SetupObject2.hpp"

namespace xo {
    using xo::scm::SetupObject2;

    Object2Appcx::Object2Appcx(const Object2Config & cfg,
                               const PrintJsonAppcx & pjson_appcx,
                               const Stringtable2Appcx & /*stringtable2_appcx*/)
    : init_evidence_{InitSubsys<S_object2_tag>::require()},
      config_{cfg}
    {
        //SetupObject2::reflect_types();

        SetupObject2::provide_json_printers(pjson_appcx.print_json());
    }
} /*namespace xo*/

/* end Object2Appcx.cpp */
