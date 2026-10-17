/** @file object2_utest_appcx.hpp
 *
 *  @author Roland Conybeare, Oct 2026
 *
 *  Access to the utest process's FacetAppcx, for tests that need one.
 *
 *  One context chain, built by object2_utest_main.cpp BEFORE any test runs.
 *  Configuring up front matters: TempArena is per-thread and takes its
 *  capacity when first used, so a chain built lazily by the first test that
 *  wants one would leave every earlier tostr() on the 4 KiB default.  Same
 *  shape as xo-printjson's printjson_utest_appcx.hpp.
 **/

#pragma once

#include <xo/facet/cx/FacetAppcx.hpp>

namespace xo {
    /** the utest process's facet context.
     *
     *  Valid for the whole of main(); tests run inside its lifetime.
     **/
    FacetAppcx & object2_utest_facet_appcx();
} /*namespace xo*/

/* end object2_utest_appcx.hpp */
