# xo-kalmanfilter/utest/CMakeLists.txt

set(SELF_EXE utest.filter)

set(SELF_SRCS
    KalmanFilter.test.cpp
    filter_utest_main.cpp)

xo_add_utest_executable(${SELF_EXE} ${SELF_SRCS})

# ----------------------------------------------------------------
# create convenience symlink from build dir back to canned data.
# This is ok since we don't implement install for unit tests

#create_symlink("${CMAKE_SOURCE_DIR}/utestdata/" "src/filter/utest/utestdata")
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/utest/utestdata)
file(CREATE_LINK ${PROJECT_SOURCE_DIR}/utest/utestdata/filter ${PROJECT_BINARY_DIR}/utest/utestdata/filter SYMBOLIC)

xo_self_dependency(${SELF_EXE} xo_kalmanfilter)
xo_dependency(${SELF_EXE} xo_statistics)
xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)

# end CMakeLists.txt
