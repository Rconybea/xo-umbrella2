/** @file Schematika.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "xo/alloc/IAlloc.hpp"

namespace xo {
    namespace scm {
        /** @class Schematika
         *  @brief schematika interpreter state
         **/
        class Schematika {
        public:
            class Impl;

            struct Config {
                /** true to enable welcome message **/
                bool welcome_flag_ = true;
                /** number of command history items to preserve **/
                std::size_t history_size = 100;
                /** on startup: load command history from this file;
                    persist last @ref history_size commands to the same file
                 **/
                std::string history_file = "scm_history.txt";
                /** when true enable console logging for repl internals **/
                bool debug_flag = false;
            };

        public:
            ~Schematika();

            /** create instance with configuration @p cfg **/
            static Schematika make(const Config & cfg);

            /** interactive read-eval-print loop.
             *  Uses replxx to read from stdin.
             *  If stdin is interactive, accepts line editing commands:
             *  - ctrl-a          goto beginning of line
             *  - ctrl-e          goto end of line
             *  - ctrl-k          delete to end of line
             *  - meta-<bs>       backwards delete word
             *  - meta-p|<up>     retrieve previous command from history
             *  - meta-n|<down>   retrieve next command from history
             *  - <pgup>/<pgdown> page through history faster
             *  - ctrl-s          forward history search
             *  - ctrl-r          backward history search
             **/
            void interactive_repl();

        private:
            Schematika(const Config & cfg);

        private:
            up<Impl> p_impl_;
        };
    }
}

/* end Schematika.hpp */
