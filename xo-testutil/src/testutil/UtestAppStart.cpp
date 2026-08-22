/** @file UtestAppStart.cpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#include "UtestAppStart.hpp"
#include "UtestConfig.hpp"
#include <xo/subsys/Subsystem.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <iostream>
#include <CLI/CLI.hpp>

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

namespace xo {
    using xo::UtestConfig;
    using xo::Subsystem;
    using xo::pp::scope;

    using std::cout;
    using std::cerr;
    using std::endl;

    int
    UtestAppStart::init(int argc, char * argv[])
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

        argv2_.push_back(argv[0]);

        remaining_ = app.remaining();

        if (help_flag) {
            // actual help impl, falls through to Session below

            cout << app_name_ << " options" << endl;
            cout << app.help() << endl;
            cout << "catch2 options" << endl;

            argv2_.push_back("--help");
        } else {
            // keep program name
            for (auto & x : remaining_)
                argv2_.push_back(x.c_str());
        }

        return 0;
    }

    void
    UtestAppStart::setup()
    {
        Subsystem::initialize_all();
    }

    int
    UtestAppStart::run()
    {
        scope log(XO_DEBUG_(UtestConfig::instance()->debug_flag()),
                  "start catch2 session");

        // run catch2's test session / help
        return Catch::Session().run(argv2_.size(), argv2_.data());
    }
} /*namespace xo*/

/* end UtestAppStart.cpp */
