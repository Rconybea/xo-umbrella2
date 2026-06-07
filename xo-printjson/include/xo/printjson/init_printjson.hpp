/* file init_printjson.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

#include "xo/subsys/Subsystem.hpp"

namespace xo {
    /* tag to represent the printjson/ subsystem within ordered initialization */
    enum S_printjson_tag {};

    /* Use:
     *   // anywhere, to declare printjson dependency  e.g. at file scope
     *   InitEvidence s_evidence = InitSubsys<S_printjson_tag>::require();
     *
     *   // from main(), though can resort to module initialization in a pybind11 library
     *   Subsystem::initialize_all();
     */
    template<>
    struct InitSubsys<S_printjson_tag> {
        static void init();
        static InitEvidence require();
    };
} /*namespace xo*/


/* end init_printjson.hpp */
