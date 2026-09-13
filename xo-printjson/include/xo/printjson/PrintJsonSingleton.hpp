/** @file PrintJsonSingleton.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "PrintJson.hpp"

namespace xo::json {

    /* Using singleton here to collect type-specific json printers,
     * collected during program initialization.
     *
     * Could relabel as PrintJsonInitContext if desired
     */
    class PrintJsonSingleton {
    public:
        static rp<PrintJson> instance();

    private:
        /* we don't need this to be stored as pointer.
         * memory burned if unused will be one empty std::vector<>
         */
        static rp<PrintJson> s_instance;
    }; /*PrintJsonSingleton*/

} /*namespace xo::json*/

/* end PrintJsonSingleton.hpp */
