/** @file FacetUtestAppcx.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "xo/facet/cx/FacetAppcx.hpp"
#include <xo/indentlog2/cx/Indentlog2Appcx.hpp>
#include <memory>

namespace xo {
    class FacetUtestAppcx {
    public:
        using UtestAppConfig = AppConfig<S_indentlog2_tag, S_facet_tag>;
        using UtestAppContext = AppContext<S_indentlog2_tag, S_facet_tag>;

        /** establish the context for this test binary.
         *
         *  Call from main() before app.run().
         *
         *  TEARDOWN.  @ref appcx_ is destroyed during static destruction,
         *  which runs AFTER the FacetRegistry/TypeRegistry singletons it
         *  refers to: those are function-local statics constructed during
         *  main(), so they are destroyed first.  That is safe only because
         *  FacetAppcx holds them by reference and no destructor in the chain
         *  dereferences them.  Should that stop being true, destroy the
         *  context explicitly at the end of main() -- while the registries
         *  are still alive -- rather than relying on this order.
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

/* end FacetUtestAppcx.hpp */
