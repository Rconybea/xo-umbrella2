/** @file WebsockUtestAppcx.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "WebsockUtestAppcx.hpp"
#include <stdexcept>

namespace xo {
    std::unique_ptr<WebsockUtestAppcx::UtestAppContext>
    WebsockUtestAppcx::appcx_;

    WebsockUtestAppcx::UtestAppContext &
    WebsockUtestAppcx::appcx()
    {
        /* not assert(): NDEBUG is set in a Release build, which would turn a
         * missing configure() into a null dereference with no message
         */
        if (!appcx_) {
            throw std::runtime_error("WebsockUtestAppcx::appcx: no context;"
                                     " main() must call configure() before app.run()");
        }

        return *appcx_;
    }

    void
    WebsockUtestAppcx::configure(const UtestAppConfig & cfg)
    {
        appcx_ = std::make_unique<UtestAppContext>(cfg);
    }
} /*namespace xo*/

/* end WebsockUtestAppcx.cpp */
