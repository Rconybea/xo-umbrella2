/** @file UtestAppStart.hpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#pragma once

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
         **/
        int run(int argc, char * argv[]);

    private:
        const char * app_name_ = "";
    };

} /*namespace xo*/

/* end UtestAppStart.hpp */
