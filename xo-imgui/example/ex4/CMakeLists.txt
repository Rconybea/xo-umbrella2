if (XO_ENABLE_EXAMPLES)
    # target executable
    set(SELF_EXE imgui_ex4)
    add_executable(${SELF_EXE} imgui_ex4.cpp
        AppState.cpp
        DrawState.cpp
        GcStateDescription.cpp
        AnimateGcCopyCb.cpp
        GenerationLayout.cpp
    )
    xo_include_options2(${SELF_EXE})

    xo_self_dependency(${SELF_EXE} xo_imgui)
    xo_dependency(${SELF_EXE} xo_object)
    xo_dependency(${SELF_EXE} randomgen)
    xo_dependency(${SELF_EXE} xo_flatstring)
    xo_dependency(${SELF_EXE} xo_alloc)
    xo_dependency(${SELF_EXE} indentlog)
endif()
