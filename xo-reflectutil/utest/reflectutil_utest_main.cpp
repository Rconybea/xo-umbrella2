/** @file reflectutil_utest_main.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

/* Self-contained catch2 main, as xo-flatstring and the other low-level
 * subsystems use -- NOT the shared UtestAppStart in libxo_testutil.
 *
 * Not a levelization constraint: xo-testutil does not depend on
 * xo-reflectutil (`xo-deps --why=xo-testutil:xo-reflectutil' finds no path).
 * It is subdirectory ORDER -- the umbrella adds xo-reflectutil before
 * xo-testutil, so the target does not exist yet when this configures.
 *
 * Nothing here needs an app context, so the cheap main is also the right one.
 */
#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

/* end reflectutil_utest_main.cpp */
