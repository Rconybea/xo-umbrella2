/** @file init_websock.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include <xo/subsys/Subsystem.hpp>

namespace xo {
    /* tag to represent the websock/ subsystem within ordered initialization */
    enum S_websock_tag {};

    /* Use:
     *   // anywhere, to declare websock dependency  e.g. at file scope
     *   InitEvidence s_evidence = InitSubsys<S_websock_tag>::require();
     *
     *   // from main(), though can resort to module initialization in a pybind11 library
     *   Subsystem::initialize_all();
     *
     * Registering json printers is WebsockAppcx's job, not init()'s -- see
     * .xo-backlog/xo-websock/issues/11
     */
    template<>
    struct InitSubsys<S_websock_tag> {
        static void init();
        static InitEvidence require();
    };
} /*namespace xo*/

/* end init_websock.hpp */
