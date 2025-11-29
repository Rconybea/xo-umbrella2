/** @file replxx.cpp **/

#include "xo/interpreter/Schematika.hpp"

int
main(int argc, char ** argv)
{
    using xo::log_level;
    using xo::scm::Schematika;

    Schematika::Config cfg;
    cfg.debug_flag = true;
    cfg.vsm_log_level_ = log_level::verbose;
    Schematika scm = Schematika::make(cfg);

    scm.interactive_repl();
}

/* end replxx.cpp */
