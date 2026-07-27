/** @file LogState.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

//#include "LogBuffer.hpp"
//#include "LogStreambuf.hpp"
#include <xo/timeutil/timeutil.hpp>

namespace xo {
    /** @brief Track per-thread state associated with indenting logger
     **/
    class LogState {
    public:
        using utc_nanos = xo::time::utc_nanos;
        using uint32_t = std::uint32_t;

    public:
        LogState(const ArenaConfig & config, bool debug_flag);

        uint32_t nesting_level() const { return nesting_level_; }

        void incr_nesting() { ++nesting_level_; }
        void decr_nesting() { --nesting_level_; }

    private:
        /** current nesting level for this thread **/
        uint32_t nesting_level_ = 0;

        /** buffer state **/
        LogBuffer logbuf_;

        /** std::streambuf implementation, storage provided by @ref logbuf_ **/
        LogStreambuf sbuf_;
    };
}

/* end LogState.hpp */
