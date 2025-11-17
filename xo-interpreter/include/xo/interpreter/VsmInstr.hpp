/** @file VsmInstr.hpp **/

#pragma once

#include <string_view>

namespace xo {
    namespace scm {
        class VirtualSchematikaMachine; // see VirtualSchematikaMachine.hpp

        /** @class VmInstr
         *  @brief Represent a particular vritual schematika machine instruction
         *
         *  A vsm instruction acts on a virtual schematika machine instance.
         **/
        class VsmInstr
        {
        public:
            using ActionFn = void (*)(VirtualSchematikaMachine * vm);

        private:
            std::string_view name_;
            //ActionFn action_;
        };
    }
}

/* end VsmInstr.hpp */
