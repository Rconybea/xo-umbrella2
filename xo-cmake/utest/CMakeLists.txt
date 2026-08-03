# xo-cmake/utest/CMakeLists.txt
#
# Python unit tests for the scripts in xo-cmake/bin.
#

find_program(XO_PYTHON3_EXECUTABLE NAMES python3)

# Skip rather than abort: this is a test for one script in bin/, and a missing
# python3 must not break configure for everything else xo-cmake installs.
# Matches how lcov/genhtml/llvm-cov are treated in ../CMakeLists.txt.
if (XO_PYTHON3_EXECUTABLE)
    add_test(
        NAME utest.xo-loc
        COMMAND ${XO_PYTHON3_EXECUTABLE} -m unittest discover
                -s ${CMAKE_CURRENT_SOURCE_DIR} -p "test_*.py" -v
    )
else()
    message(WARNING "xo-cmake/utest: python3 not found, skipping xo-loc tests")
endif()

# end CMakeLists.txt
