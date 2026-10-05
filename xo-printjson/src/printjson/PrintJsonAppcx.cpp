/** @file PrintJsonAppcx.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "cx/PrintJsonAppcx.hpp"
#include "PrintJsonSingleton.hpp"
#include "PrintJson.hpp"

namespace xo {
    using xo::json::PrintJsonSingleton;

    PrintJsonAppcx::PrintJsonAppcx(const PrintJsonConfig & cfg,
                                   const ReflectAppcx & reflect_appcx)
        : init_evidence_{InitSubsys<S_printjson_tag>::require()},
          config_{cfg}
#ifdef NOT_USING
          reflect_appcx_{reflect_appcx}
#endif
    {
        json::PrintJson::reflect_self(reflect_appcx.type_table());

        this->print_json_ = PrintJsonSingleton::instance();
        /* the singleton: an application has one PrintJson */
        this->print_json_->assign_max_depth(cfg.max_depth_);
    }

} /*namespace xo*/

/* end PrintJsonAppcx.cpp */
