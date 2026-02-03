/** @file ErrorArena.hpp
*
 *  @author Roland Conybeare, Feb 2026
 **/

#pragma once

#include "DArena.hpp"

namespace xo {
    namespace mm {

        /** @brief Dedicated arena for error reporting
         *
         *  Reserving memory for error messaages.
         *  Motivation
         *  1. so we have room to report an out-of-memory condition
         *  2. so we have place to allocate for an error that
         *     doesn't interfere with other allocator state
         *
         *  Expect to reset arena between errors, so only need
         *  enough room to report one error.
         *
         *  To initialize explicitly:
         *  @code
         *   // before any other ErrorArena method calls:
         *   ErrorArena::init_once(cfg...);
         *
         *   // do stuff with ErrorArena..
         *   ErrorArena::instance()
         *  @endcode
         *
         *  Reminder: can't use obj<AAllocator> here,
         *            would be leveling violation.
         **/
        class ErrorArena {
        public:
            /** default configuration for error arena **/
            static ArenaConfig default_config();

            /** idempotent initialization **/
            static void init_once(const ArenaConfig & cfg = default_config());

            /** get initialized instnace **/
            static DArena * instance();

        private:
            static DArena s_instance;
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end ErrorArena.hpp */
