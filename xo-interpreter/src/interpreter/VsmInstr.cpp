/** @file VsmInstr.cpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#include "VsmInstr.hpp"

namespace xo {
    namespace scm {
        VsmInstr::VsmInstr(Opcode opcode,
                           std::string_view name) : opcode_{opcode}, name_{name}
        {}
    }
}

/* end VsmInstr.cpp */
