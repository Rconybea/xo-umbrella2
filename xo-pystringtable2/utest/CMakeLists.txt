# xo-pystringtable2/utest/CMakeLists.txt
#
# Python tests for xo.stringtable2's subsystem-configuration contract.
#
# Here rather than in a suite further up: xo-pystringtable2 is 26 in
# subsystem-list, so a stringtable2 test living in xo-pyobject2/utest (28)
# would report a stringtable2 regression two levels up, and
# `xo-build --utest xo-pystringtable2' would pass having tested nothing.
#
# xo_add_python_utest() writes an executable script beside the c++ utest
# binaries and points ctest at it, so these tests can be run and filtered by
# hand the same way:
#
#   .build/xo-pystringtable2/utest/utest.pystringtable2
#   .build/xo-pystringtable2/utest/utest.pystringtable2 test_pystringtable2.ConfigurationTestCase

if (ENABLE_TESTING)
    xo_add_python_utest(utest.pystringtable2 ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# end CMakeLists.txt
