/** @file PrintJsonSingleton.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "PrintJsonSingleton.hpp"

namespace xo::json {

    rp<PrintJson>
    PrintJsonSingleton::s_instance;

    rp<PrintJson>
    PrintJsonSingleton::instance()
    {
        if (!s_instance)
            s_instance = new PrintJson();

        return s_instance;
    } /*instance*/

} /*namespace xo::json*/

/* end PrintJsonSingleton.cpp */
