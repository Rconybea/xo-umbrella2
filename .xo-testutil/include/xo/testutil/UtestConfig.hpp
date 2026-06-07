/** @file UtestConfig.hpp
 *
 *  @author Roland Conybeare, May 2026
 **/

namespace xo {

    /** unit-test configuration here
     *
     *  TODO: promote to its own library, along with UtestListener
     **/
    struct UtestConfig {
        bool debug_flag() const { return debug_flag_; }

        /** announce each test using catch2's listener api **/
        bool announce_flag_ = false;
        /** enable debug output for all (!) tests **/
        bool debug_flag_ = false;

        static UtestConfig * instance();
    };

}

/* end UtestConfig.hpp */
