# xo-pyreactor2websock/utest/CMakeLists.txt
#
# Python smoke test for xo.reactor2websock.  Run by hand as
#
#   .build/xo-pyreactor2websock/utest/utest.pyreactor2websock

if (ENABLE_TESTING)
    xo_add_python_utest(utest.pyreactor2websock ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# end CMakeLists.txt
