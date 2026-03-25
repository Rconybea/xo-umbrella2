/** @file MutationLogEntry.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "MutationLogEntry.hpp"

namespace xo {
    namespace mm {

        MutationLogEntry::MutationLogEntry(void * parent,
                                           void ** p_data,
                                           obj<AGCObject> snap)
                : parent_{parent},
                  p_data_{p_data},
                  snap_{snap}
        {}

    } /*namespace mm*/
} /*namespace xo*/

/* end MutationLogEntry.cpp */
