# xo-cmake/utest/CMakeLists.txt
#
# Python unit tests for the scripts in xo-cmake/bin.
#

find_program(XO_PYTHON3_EXECUTABLE NAMES python3)

if (NOT XO_PYTHON3_EXECUTABLE)
    message(FATAL_ERROR "xo-cmake/utest: python3 not found, needed for xo-loc tests")
endif()

add_test(
    NAME utest.xo-loc
    COMMAND ${XO_PYTHON3_EXECUTABLE} -m unittest discover
            -s ${CMAKE_CURRENT_SOURCE_DIR} -p "test_*.py" -v
)

# end CMakeLists.txt
