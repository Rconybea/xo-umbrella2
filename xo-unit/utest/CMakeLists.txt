# xo-unit/utest/CMakeLists.txt

set(SELF_EXE utest.unit)
set(SELF_SRCS
    unit_utest_main.cpp  #mpl_unit.test.cpp
    xquantity.test.cpp
    quantity.test.cpp
    bpu.test.cpp
    basis_unit.test.cpp
    scaled_unit.test.cpp
    natural_unit.test.cpp
    unit.test.cpp #quantity.test.cpp
)

if (ENABLE_TESTING)
    xo_add_utest_executable(${SELF_EXE} ${SELF_SRCS})

    # ----------------------------------------------------------------
    # dependencies..

    xo_self_dependency(${SELF_EXE} xo_unit)
    xo_headeronly_dependency(${SELF_EXE} xo_ratio)
    # at least need explicit {indentlog, randomgen} deps in a submodule build
    xo_headeronly_dependency(${SELF_EXE} indentlog)
    xo_headeronly_dependency(${SELF_EXE} randomgen)
    xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)
endif()

# end CMakeLists.txt
