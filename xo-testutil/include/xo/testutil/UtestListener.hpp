/** @file UtestListener.hpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#pragma once

#include "UtestConfig.hpp"

// note: caller must define CATCH_CONFIG_EXTERNAL_INTERFACES (not
//       CATCH_CONFIG_RUNNER/CATCH_CONFIG_MAIN) before including this header:
//       we need Catch::TestEventListenerBase + CATCH_REGISTER_LISTENER, but
//       the catch2 runtime implementation must be compiled exactly once, in
//       libxo_testutil (UtestAppStart.cpp).  Defining CATCH_CONFIG_RUNNER in a
//       test executable compiles a second copy of the runtime -> a separate
//       test registry the runner never sees (esp. on osx; see UtestAppStart).
#include <catch2/catch.hpp>
#include <iostream>

namespace xo {

    /** @brief listener for catch2 unit tests.
     *  catch2 invokes this at the beginning of each unit test
     *
     *  Enable with:
     *  @begin_code
     *    #include <catch2/catch.hpp>
     *    CATCH_REGISTER_LISTENER(UtestListener);
     *  @end_code
     **/
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
}

/* end UtestListener.hpp */
