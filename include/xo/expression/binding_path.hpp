/* file binding_path.hpp
 *
 * author: Roland Conybeare, Jul 2024
 */

#pragma once

namespace xo {
    namespace ast {
        /** @class path
         *
         *  @brief path from the *use* of a variable to the environment
         *  providing its location.
         **/
        struct binding_path {
            /** @of parent links to traverse.  -1 if global **/
            int i_link_ = -1;
            /** for variables bound in some local environment:
             *  slot# within that environment.
             *
             *  Ignored if @ref i_link_ is -1
             **/
            int j_slot_ = 0;
        }; /*binding_path*/
    } /*namespace ast*/
} /*namespace xo*/


/* end binding_path.hpp */
