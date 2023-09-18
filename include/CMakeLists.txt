# header-only library
add_library(indentlog INTERFACE)
target_include_directories(indentlog INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )
