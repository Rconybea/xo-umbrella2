# xo-pyprintjson/utest/CMakeLists.txt
#
# Python tests for xo.printjson's subsystem-configuration contract.
#
# Here rather than in xo-pyobject2/utest, which already imports the rest of the
# stack: xo-pyprintjson is 24 in subsystem-list and xo-pyobject2 is 27, so a
# printjson test living there would report a printjson regression three levels
# up, and `xo-build --utest xo-pyprintjson' would pass having tested nothing.
#
# xo_add_python_utest() writes an executable script beside the c++ utest
# binaries and points ctest at it, so these tests can be run and filtered by
# hand the same way:
#
#   .build/xo-pyprintjson/utest/utest.pyprintjson
#   .build/xo-pyprintjson/utest/utest.pyprintjson test_pyprintjson.ConfigurationTestCase

if (ENABLE_TESTING)
    xo_add_python_utest(utest.pyprintjson ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# end CMakeLists.txt
