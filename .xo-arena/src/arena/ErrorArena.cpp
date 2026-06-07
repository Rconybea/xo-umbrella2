/** @file ErrorArena.cpp
*
 *  @author Roland Conybeare, Feb 2026
 **/

#include "ErrorArena.hpp"

namespace xo {
    namespace mm {
        DArena
        ErrorArena::s_instance;

        ArenaConfig
        ErrorArena::default_config()
        {
            return ArenaConfig().with_name("error-arena").with_size(16 * 1024);
        }

        namespace  {
            bool s_init_done = false;
        }

        void
        ErrorArena::init_once(const ArenaConfig & cfg)
        {
            if (!s_init_done) {
                s_init_done = true;
                s_instance = DArena::map(cfg);
            }
        }

        DArena *
        ErrorArena::instance()
        {
            init_once(default_config());

            return &s_instance;
        }
        
    } /*namespace mm*/
} /*namespace xo*/

/* end ErrorArena.cpp */
