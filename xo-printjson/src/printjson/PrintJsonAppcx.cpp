/** @file PrintJsonAppcx.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "cx/PrintJsonAppcx.hpp"
#include "PrintJsonSingleton.hpp"

namespace xo {
    using xo::json::PrintJsonSingleton;

    PrintJsonAppcx::PrintJsonAppcx(const PrintJsonConfig & cfg,
                                   const ReflectAppcx & reflect_appcx)
        : init_evidence_{InitSubsys<S_printjson_tag>::require()},
          config_{cfg},
          reflect_appcx_{reflect_appcx}
    {
        this->print_json_ = PrintJsonSingleton::instance();
    }

} /*namespace xo*/

/* end PrintJsonAppcx.cpp */
