/** @file printjson_utest_appcx.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  Access to the utest process's FacetAppcx, for tests that need one.
 *
 *  Why an accessor rather than each test file owning a context: an
 *  Indentlog2Appcx maps a temp arena and installs a PrettySinkFactory, so a
 *  second one is not free and not obviously harmless.  There is one chain, it
 *  is built by printjson_utest_main.cpp, and this is how a test reaches it.
 *
 *  ObjectSlotJson.test.cpp needs it because a non-empty ObjectSlot can only be
 *  made by a DHandleStore, which requires FacetAppcxCreated evidence -- see
 *  .xo-backlog/xo-facet/issues/04.
 **/

#pragma once

#include <xo/facet/cx/FacetAppcx.hpp>

namespace xo {
    /** the utest process's facet context.
     *
     *  Valid for the whole of main(); tests run inside its lifetime.
     **/
    FacetAppcx & printjson_utest_facet_appcx();
} /*namespace xo*/

/* end printjson_utest_appcx.hpp */
