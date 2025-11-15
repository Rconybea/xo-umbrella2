/** @file VirtualSchematikaMachine.hpp **/

#pragma once

#include "VsmInstr.hpp"

namespace xo {
    namespace scm {
        /** @class VirtualSchematikaMachine
         *  @brief Virtual machine implementing a Schematika interpreter
         *
         **/
        class VirtualSchematikaMachine {
        public:
            VirtualSchematikaMachine();


        private:
            /** program counter **/
            const VsmInstr * pc_ = nullptr;


        };

    } /*namespace scm*/
} /*namespace xo*/

/* end VirtualSchematikaMachine.hpp */
