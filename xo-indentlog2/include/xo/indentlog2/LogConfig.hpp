/** @file LogConfig.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include <cstdint>

namespace xo {
    /** @brief configuration for Logger **/
    class LogConfig {
    public:
        using uint32_t = std::uint32_t;

        LogConfig();

        static LogConfig & instance() { return s_instance; }

        /** number of spaces per nesting level.  0 for no nesting **/
        uint32_t indent_width = 2;

        /** enable explicit nesting count **/
        bool nesting_count_enabled = true;

    private:
        static LogConfig s_instance;
    };

} /*namespace xo*/

/* end LogConfig.hpp */
