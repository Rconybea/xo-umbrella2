# imgui/CMakeLists.txt

find_path(IMGUI_INCLUDE_DIR
    NAMES imgui/imgui.h
    DOC "path to imgui header")
if (IMGUI_INCLUDE_DIR)
    message(STATUS
        "found imgui/imgui.h in IMGUI_INCLUDE_DIR=[${IMGUI_INCLUDE_DIR}]")
else()
    message(FATAL_ERROR "unable to find imgui.h")
endif()

set(SELF_LIB xo_imgui)
set(SELF_SRCS
    VulkanApp.cpp
    ImRect.cpp
        ${IMGUI_INCLUDE_DIR}/imgui/imgui.cpp
        ${IMGUI_INCLUDE_DIR}/imgui/imgui_demo.cpp
        ${IMGUI_INCLUDE_DIR}/imgui/imgui_draw.cpp
        ${IMGUI_INCLUDE_DIR}/imgui/imgui_widgets.cpp
        ${IMGUI_INCLUDE_DIR}/imgui/imgui_tables.cpp
        ${IMGUI_INCLUDE_DIR}/imgui/backends/imgui_impl_sdl2.cpp
        #${IMGUI_INCLUDE_DIR}/imgui/backends/imgui_impl_opengl3.cpp
        #${IMGUI_INCLUDE_DIR}/imgui/backends/imgui_impl_glfw.cpp
        ${IMGUI_INCLUDE_DIR}/imgui/backends/imgui_impl_vulkan.cpp)

xo_add_shared_library4(
    ${SELF_LIB}
    ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS}
)

find_package(Vulkan REQUIRED)
target_link_libraries(${SELF_LIB} PUBLIC Vulkan::Vulkan)

xo_external_pkgconfig_dependency(${SELF_LIB} SDL2 sdl2)

# NOTE: would prefer to use just IMGUI_INCLUDE_DIR,
# but imgui/backends/ .h files don't quote the imgui/ stem
#
target_include_directories(${SELF_LIB} PUBLIC ${IMGUI_INCLUDE_DIR}/imgui)

xo_dependency(${SELF_LIB} indentlog)
