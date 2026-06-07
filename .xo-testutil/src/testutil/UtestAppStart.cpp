/** @file UtestAppStart.cpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#include "UtestAppStart.hpp"
#include "UtestConfig.hpp"
#include <xo/subsys/Subsystem.hpp>
#include <xo/indentlog/scope.hpp>
#include <CLI/CLI.hpp>

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

namespace xo {
    using xo::UtestConfig;
    using xo::scope;
    using xo::xtag;

    using std::cout;
    using std::cerr;
    using std::endl;

    int
    UtestAppStart::run(int argc, char * argv[])
    {
        CLI::App app{app_name_};

        app.set_help_flag(); // disable default help impl, see below
        {
            app.add_flag("--debug",
                         UtestConfig::instance()->debug_flag_,
                         "enable debug logging (for all tests)");

            app.add_flag("--announce",
                         UtestConfig::instance()->announce_flag_,
                         "announce each test via UtestListener");
        }

        bool help_flag = false;
        {
            app.add_flag("--help,-h,-?", help_flag, "print this help message and exit");
        }

        app.allow_extras();
        CLI11_PARSE(app, argc, argv);

        std::vector<const char *> argv2 = {argv[0]};

        if (help_flag) {
            // actual help impl, falls through to Session below

            cout << app_name_ << " options" << endl;
            cout << app.help() << endl;
            cout << "catch2 options" << endl;

            argv2.push_back("--help");
        } else {
            // keep program name
            for (auto & x : app.remaining())
                argv2.push_back(x.c_str());
        }

        using xo::Subsystem;
        Subsystem::initialize_all();

        scope log(XO_DEBUG(UtestConfig::instance()->debug_flag()),
                  "start catch2 session");

        // run catch2's test session / help
        return Catch::Session().run(argv2.size(), argv2.data());
    }
} /*namespace xo*/

/* end UtestAppStart.cpp */
