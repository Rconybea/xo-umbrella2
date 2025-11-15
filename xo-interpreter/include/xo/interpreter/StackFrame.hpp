/** @file StackFrame.hpp **/

#include "xo/alloc/Object.hpp"

namespace xo {
    namespace scm {
        /** @class StackFrame
         *  @brief Represent a single runtime stack frame for a Schematika function
         *
         *  StackFrame intended to be used for interpreted functions.
         *  Compiled functions will stil likely have stack frames, but need not use the
         *  @ref StackFrame class
         **/
        class StackFrame : public Object {
        public:
            StackFrame(std::size_t n_slot)

        private:

        };
    } /*namespace scm*/
} /*namespace xo*/

/* end StackFrame.hpp */
