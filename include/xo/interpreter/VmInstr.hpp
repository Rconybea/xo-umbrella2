/** @file VmInstr.hpp **/

#pragma once

#include <string>

namespace xo {
    namespace scm {
        class VirtualSchematikaMachine; // see VirtualSchematikaMachine.hpp

        /** @class VmInstr
         *  @brief Represent a particular vritual schematika machine instruction
         *
         *  A vsm instruction acts on a virtual schematika machine instance.
         **/
        class VmInstr
        {
        public:
            using ActionFn = void (*)(VirtualSchematikaMachine * vm);

        private:
            std::string name_;
            ActionFn action_;
        };
    }
}

/* end VmInstr.hpp */
