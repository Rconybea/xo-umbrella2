/** @file Schematika.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pramga once



namespace xo {
    namespace scm {

        /** @class Schematika
         *  @brief schematika interpreter state
         **/
        class Schematika {
        public:
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
            class Impl;
            std::unique_ptr<Impl> p_impl_;
        };
    }
}

/* end Schematika.hpp */
