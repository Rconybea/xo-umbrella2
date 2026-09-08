# xo-pyobject2/utest/CMakeLists.txt
#
# Python tests for the extension stack.

# The generated wrapper is what puts the extension modules on PYTHONPATH, so
# running the tests through it tests that too.  CMAKE_BINARY_DIR is the top of
# the build tree either way -- the umbrella's wrapper in an umbrella build, this
# subsystem's own in a standalone one -- so no branch is needed here.
set(XO_PYTHON_WRAPPER ${CMAKE_BINARY_DIR}/xo-python)

# Skip rather than abort when python3 is missing: matches xo-cmake/utest, and a
# missing interpreter must not break configure for the library itself.
find_program(XO_PYTHON3_EXECUTABLE NAMES python3)

if (ENABLE_TESTING AND XO_PYTHON3_EXECUTABLE)
    add_test(
        NAME utest.pyobject2
        COMMAND ${XO_PYTHON_WRAPPER} -m unittest discover
                -s ${CMAKE_CURRENT_SOURCE_DIR} -p "test_*.py" -v
    )
elseif (ENABLE_TESTING)
    message(WARNING "xo-pyobject2/utest: python3 not found, skipping python tests")
endif()

# end CMakeLists.txt
