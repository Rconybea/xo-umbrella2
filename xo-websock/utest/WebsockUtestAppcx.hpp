/** @file WebsockUtestAppcx.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "xo/websock/cx/WebsockAppcx.hpp"
#include <xo/printjson/cx/PrintJsonAppcx.hpp>
#include <xo/reflect/cx/ReflectAppcx.hpp>
#include <xo/indentlog2/cx/Indentlog2Appcx.hpp>
#include <memory>

namespace xo {
    /** the context for xo-websock's test binaries (utest.websock,
     *  utest.websock.live).  Tests make Webservers from
     *  @c appcx().cx<S_websock_tag>() -- Webserver::make takes a WebsockAppcx
     *  (.xo-backlog/xo-websock/issues/11).  Same shape as FacetUtestAppcx
     *  (xo-facet/utest).
     **/
    class WebsockUtestAppcx {
    public:
        /* websock is built on printjson, printjson on reflect, reflect on
         * indentlog2
         */
        using UtestAppConfig = AppConfig<S_indentlog2_tag,
                                         S_reflect_tag,
                                         S_printjson_tag,
                                         S_websock_tag>;
        using UtestAppContext = AppContext<S_indentlog2_tag,
                                           S_reflect_tag,
                                           S_printjson_tag,
                                           S_websock_tag>;

        /** establish the context for this test binary.
         *
         *  Call from main() before app.run().
         *
         *  TEARDOWN.  @ref appcx_ is destroyed during static destruction.
         *  WebsockAppcx holds its PrintJson by rp<>, keeping it alive; no
         *  destructor in the chain sends or prints.  Should that stop being
         *  true, destroy the context explicitly at the end of main().
         **/
        static void configure(const UtestAppConfig & cfg);

        /** the context established by @ref configure.
         *  Throws if main() has not called configure() yet.
         **/
        static UtestAppContext & appcx();

    private:
        static std::unique_ptr<UtestAppContext> appcx_;
    };
} /*namespace xo*/

/* end WebsockUtestAppcx.hpp */
