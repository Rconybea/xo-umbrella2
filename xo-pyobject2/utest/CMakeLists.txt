# xo-pyobject2/utest/CMakeLists.txt
#
# Python tests for the extension stack.
#
# xo_add_python_utest() writes an executable script beside the c++ utest
# binaries and points ctest at it, so these tests can be run and filtered by
# hand the same way:
#
#   .build/xo-pyobject2/utest/utest.pyobject2
#   .build/xo-pyobject2/utest/utest.pyobject2 test_pyobject2.VisitPoolsTestCase
#
# The script runs them through the generated xo-python wrapper, which is what
# puts the extension modules on PYTHONPATH -- so running the tests exercises
# that wiring too.

if (ENABLE_TESTING)
    xo_add_python_utest(utest.pyobject2 ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# end CMakeLists.txt
