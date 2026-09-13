# xo-pyreflect/utest/CMakeLists.txt
#
# Python tests for xo.reflect's subsystem-configuration contract.
#
# Here rather than in xo-pyobject2/utest, which already imports the rest of the
# stack: xo-pyreflect is 21 in subsystem-list and xo-pyobject2 is 27, so a
# reflect test living there would report a reflect regression six levels up,
# and `xo-build --utest xo-pyreflect' would pass having tested nothing.
#
# xo_add_python_utest() writes an executable script beside the c++ utest
# binaries and points ctest at it, so these tests can be run and filtered by
# hand the same way:
#
#   .build/xo-pyreflect/utest/utest.pyreflect
#   .build/xo-pyreflect/utest/utest.pyreflect test_pyreflect.ConfigurationTestCase

if (ENABLE_TESTING)
    xo_add_python_utest(utest.pyreflect ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# end CMakeLists.txt
