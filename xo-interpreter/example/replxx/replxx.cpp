/** @file replxx.cpp **/

#include "xo/interpreter/Schematika.hpp"

int
main(int argc, char ** argv)
{
    using xo::scm::Schematika;

    Schematika::Config cfg;
    Schematika scm = Schematika::make(cfg);

    scm.interactive_repl();
}

/* end replxx.cpp */
