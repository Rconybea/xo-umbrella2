/** @file Interpreter2Appcx.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "cx/Interpreter2Appcx.hpp"

namespace xo {
    Interpreter2Appcx::Interpreter2Appcx(const Interpreter2Config & cfg)
      : init_evidence_{InitSubsys<S_interpreter2_tag>::require()},
        config_{cfg}
    {}
} /*namespace xo*/

/* end Interpreter2Appcx.cpp */
