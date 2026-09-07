/** @file FacetUtestAppcx.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "FacetUtestAppcx.hpp"
#include <stdexcept>

namespace xo {
    std::unique_ptr<FacetUtestAppcx::UtestAppContext>
    FacetUtestAppcx::appcx_;

    FacetUtestAppcx::UtestAppContext &
    FacetUtestAppcx::appcx()
    {
        /* not assert(): NDEBUG is set in a Release build
         * (CMAKE_CXX_FLAGS_RELEASE is "-O3 -DNDEBUG"), which would turn a
         * missing configure() into a null dereference with no message
         */
        if (!appcx_) {
            throw std::runtime_error("FacetUtestAppcx::appcx: no context;"
                                     " main() must call configure() before app.run()");
        }

        return *appcx_;
    }

    void
    FacetUtestAppcx::configure(const UtestAppConfig & cfg)
    {
        appcx_ = std::make_unique<UtestAppContext>(cfg);
    }
} /*namespace xo*/

/* end FacetUtestAppcx.cpp */
