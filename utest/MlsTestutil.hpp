/** @file MlsTestutil.hpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

#include <xo/gc/MutationLogStore.hpp>

namespace ut {
    class MlsTestutil {
    public:
        using MutationLogStore = xo::mm::MutationLogStore;
        using Generation = xo::mm::Generation;

        static void verify_fromspace_only_logged(const MutationLogStore & mls,
                                                 Generation upto);
        static void verify_tospace_only_logged(const MutationLogStore & mls,
                                               Generation upto);
        /** verify that each mutation log entry is either:
         *  1. invalid.   cached destination no longer current
         *  2. necessary: source age > dest age
         **/
        static void verify_mlog_load_bearing(const MutationLogStore & mls,
                                             Generation upto);
    };
}
