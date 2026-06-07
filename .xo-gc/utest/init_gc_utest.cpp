/** @file init_gc_utest.cpp
*
 *  @author Roland Conybeare, Apr 2026
 **/

#include "init_gc_utest.hpp"
#include "MockCollector.hpp"
#include <xo/gc/init_gc.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/indentlog/scope.hpp>

namespace xo {
    using xo::mm::SetupGcUtest;
    using xo::facet::FacetRegistry;
    //using xo::reflect::typeseq;

    bool
    SetupGcUtest::register_facets()
    {
        scope log(XO_DEBUG(false));

        FacetRegistry::register_impl<ACollector, DMockCollector>();

        //log && log(xtag("DMockCollector.tseq", typeseq::id<DMockCollector>()));

        return true;
    }

    void
    InitSubsys<S_gc_utest_tag>::init()
    {
        SetupGcUtest::register_facets();
    }

    InitEvidence
    InitSubsys<S_gc_utest_tag>::require()
    {
        InitEvidence retval;

        /* recursive subsystem deps for xo-gc/utest */
        retval ^= InitSubsys<S_gc_tag>::require();

        /* xo-gc/utest/'s own initialization code */
        retval ^= Subsystem::provide<S_gc_utest_tag>("gc-utest", &init);

        return retval;
    }

}
