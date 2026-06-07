if (XO_ENABLE_EXAMPLES)
    # imgui dependency

    if (XO_ENABLE_OPENGL)

        # TODO: maybe incorporate imgui sources into xo-imgui library
        find_path(IMGUI_INCLUDE_DIR
            NAMES imgui/imgui.h
            DOC "path to imgui header"
            HINTS ${PROJECT_SOURCE_DIR}/include
        )
        if (IMGUI_INCLUDE_DIR)
            message(STATUS "found imgui/imgui.h in IMGUI_INCLUDE_DIR=[${IMGUI_INCLUDE_DIR}]")
        else()
            message(FATAL_ERROR "unable to find imgui.h")
        endif()

        # target executable

        set(SELF_EXE imgui_ex1)
        add_executable(${SELF_EXE} imgui_ex1.cpp
                    ${IMGUI_INCLUDE_DIR}/imgui/imgui.cpp
                    ${IMGUI_INCLUDE_DIR}/imgui/imgui_demo.cpp
                    ${IMGUI_INCLUDE_DIR}/imgui/imgui_draw.cpp
                    ${IMGUI_INCLUDE_DIR}/imgui/imgui_widgets.cpp
                    ${IMGUI_INCLUDE_DIR}/imgui/imgui_tables.cpp
                    ${IMGUI_INCLUDE_DIR}/imgui/backends/imgui_impl_sdl2.cpp
                    ${IMGUI_INCLUDE_DIR}/imgui/backends/imgui_impl_opengl3.cpp
        )
        xo_include_options2(${SELF_EXE})

        # OpenGL dependency

        # have to choose between
        #   libGL.so
        # or
        #   libOpenGL.so + libGLX.so    # GLVND = OpenGL Vendor Neutral Dispatch (e.g. mesa|nvda)
        #
        # expect in .build/CMakeCache.txt:
        #   OPENGL_opengl_LIBRARY:FILEPATH=/path/to/libOpenGL.so
        #   OpenGL_DIR:PATH=OpenGL_DIR-NOTFOUND  # no cmake config file

        set(OpenGL_GL_PREFERENCE GLVND)  # instead of default LEGACY
        find_package(OpenGL REQUIRED)  # find_package(OpenGL CONFIG REQUIRE) won't work

        target_link_libraries(${SELF_EXE} PUBLIC OpenGL::GL)
        #    target_link_libraries(${SELF_EXE} PUBLIC ${OPENGL_opengl_LIBRARY})
        #    target_include_directories(${SELF_EXE} PUBLIC OpenGL_INCLUDE_DIRS)
        #target_compile_options(${SELF_EXE} PUBLIC OpenGL_CFLAGS_OTHER)
        #xo_external_dependency(${SELF_EXE} OpenGL)

        # GLEW dependency
        #  GLEW = opengl extension wrangler
        #  cross-platform opengl extension loader
        # not needed if using vulkan

        xo_external_pkgconfig_dependency(${SELF_EXE} GLEW glew)

        # GLFW dependency

        #    find_package(glfw3 CONFIG REQUIRED)
        #    target_link_libraries(${SELF_EXE} PUBLIC glfw) # want -lglfw

        # SDL2 dependency

        xo_external_pkgconfig_dependency(${SELF_EXE} SDL2 sdl2)

        # would prefer to use jsut IMGUI_INCLUDE_DIR,
        # but imgui/backends/ .h files don't quote the imgui/ stem
        #
        target_include_directories(${SELF_EXE} PUBLIC ${IMGUI_INCLUDE_DIR}/imgui)

        install(TARGETS ${SELF_EXE} DESTINATION ${XO_EXAMPLE_INSTALL_DIR})
    endif()
endif()
