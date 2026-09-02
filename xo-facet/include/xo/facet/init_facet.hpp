/** @file init_facet.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include <xo/indentlog2/init_indentlog2.hpp>
#include <xo/subsys/Subsystem.hpp>
#include <xo/ppsink/scope.hpp>

namespace xo {
    /** tag to represent the xo-facet/ subystem with ordered initialization **/
    enum S_facet_tag {};

    template <>
    struct InitSubsys<S_facet_tag> {
    public:
        using scope = xo::pp::scope;

    public:
        static void init() {}

        static InitEvidence require() {
            scope log(XO_DEBUG_(true));

            InitEvidence retval;

            /* direct subsystem deps fro xo-facet/ */
            retval ^= InitSubsys<S_indentlog2_tag>::require();

            retval ^= Subsystem::provide<S_facet_tag>("facet", &init);

            return retval;
        }
    };
} /*namespace xo*/

/* end init_facet.hpp */
