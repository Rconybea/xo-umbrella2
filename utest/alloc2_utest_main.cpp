/* file alloc2_utest_main.cpp */

#include "TestUtil.hpp"
#include <xo/subsys/Subsystem.hpp>
#include <xo/indentlog/scope.hpp>
#include <CLI/CLI.hpp>

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

namespace xo {

    struct UtestListener : Catch::TestEventListenerBase {
        using TestEventListenerBase::TestEventListenerBase;

        // TestCasweInfo members: .name, .className, .description, .tags, lineInfo {.file, .line}
        virtual void testCaseStarting(const Catch::TestCaseInfo & info) override {
            using std::cerr;
            using std::endl;

            // preamble

            if (UtestConfig::instance()->announce_flag_) {
                cerr << "Starting unit test: "
                << "[" << info.name << "]"
                << " at "
                << "[" << info.lineInfo.file << ":" << info.lineInfo.line << "]"
                << endl;
            }
        }

        virtual void testCaseEnded(const Catch::TestCaseStats & stats) override {
            // postamble
        }

        // also sectionStarting / sectionEnded

    };

    CATCH_REGISTER_LISTENER(UtestListener);
}

int
main(int argc, char* argv[])
{
    using xo::UtestConfig;
    using xo::scope;
    using xo::xtag;

    using std::cout;
    using std::cerr;
    using std::endl;

    //cerr << xtag("cli11", CLI11_VERSION) << endl; // version 2.5.0

    CLI::App app{"utest.alloc2: xo-alloc2 unit tests"};
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

        cout << "utest.alloc2 options" << endl;
        cout << app.help() << endl;
        cout << "catch2 options" << endl;

        argv2.push_back("--help");
    } else {
        // keep program name
        for (auto & x : app.remaining())
            argv2.push_back(x.c_str());

        using xo::Subsystem;
        Subsystem::initialize_all();

    }

    scope log(XO_DEBUG(UtestConfig::instance()->debug_flag()), "start catch2 session");

    // run catch2's test session / help
    return Catch::Session().run(argv2.size(), argv2.data());
}

/* end alloc2_utest_main.cpp */
