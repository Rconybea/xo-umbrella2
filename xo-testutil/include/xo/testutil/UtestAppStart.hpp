/** @file UtestAppStart.hpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#pragma once

#include <string>
#include <vector>

namespace xo {

    /** @brief Startup sequence for a unit test
     *
     *  Standard unit test startup sequence
     **/
    class UtestAppStart {
    public:
        explicit UtestAppStart(const char * app_name) : app_name_{app_name} {}

        /**
         *  Parse program arguments; recognize XO test arguments,
         *  sending remainder to catch2; do subsystem initialization
         *
         *  returns -1 on parsing failure
         **/
        int init(int argc, char * argv[]);

        /** alloc global resources, if any **/
        void setup();

        /** run unit test **/
        int run();

    private:
        /** Application name **/
        const char * app_name_ = "";
        /** Owns the strings in argv2_[].
         *  Necessary because argv2_ is constructed from cli11 app.remaining(),
         *  which returns by value.
         **/
        std::vector<std::string> remaining_;
        /** args remaining after parsing --debug, --announce, --help|-h|-? **/
        std::vector<const char *> argv2_;
    };

} /*namespace xo*/

/* end UtestAppStart.hpp */
