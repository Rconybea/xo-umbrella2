cmake_minimum_required(VERSION 3.10)

project(nestlog VERSION 0.1)
enable_language(CXX)
enable_testing()

include(cmake/nestlog.cmake)

add_subdirectory(include)
add_subdirectory(example)

# this doesn't work in include/CMakeLists.txt
install(TARGETS indentlog DESTINATION include)

set(CMAKE_INSTALL_PREFIX /home/roland/local)
set(CMAKE_INSTALL_RPATH /home/roland/local/lib)
