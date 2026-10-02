/** @file WebsockAppcx.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "cx/WebsockAppcx.hpp"
#include "websock_json.hpp"
#include "websock_reflect.hpp"

namespace xo {
    namespace {
        constexpr std::uint64_t c_websockappcx_creation_secret = 0x5eb50c;
    }

    WebsockAppcx::WebsockAppcx(const WebsockConfig & cfg,
                               const ReflectAppcx & reflect_appcx,
                               const PrintJsonAppcx & printjson_appcx)
        : init_evidence_{InitSubsys<S_websock_tag>::require()},
          websock_evp_{c_websockappcx_creation_secret},
          config_{cfg},
          print_json_{printjson_appcx.print_json()}
    {
        /* types first: a printer is found by its type's TypeDescr */
        web::websock_reflect_types(reflect_appcx.type_table());
        web::provide_websock_json_printers(this->print_json_.get());
    }
} /*namespace xo*/

/* end WebsockAppcx.cpp */
