/* xo-imgui/example/ex1/imgui_ex1.cpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include <SDL.h>
#include <GL/glew.h>

#include "SDL_events.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

//#include <GLFW/glfw3.h>

#ifdef NOPE
#include <SDL_opengl.h>
#endif

#include <iostream>
#include <unistd.h>

int main(int, char **)
{
    using namespace std;

    std::cout << "Hello, world!" << std::endl;

    SDL_SetHint(SDL_HINT_VIDEO_X11_FORCE_EGL, "1");

    SDL_Init(SDL_INIT_VIDEO);

    SDL_version compiled;
    SDL_VERSION(&compiled);
    std::cerr << "SDL version: "
              << (int)compiled.major
              << "." << (int)compiled.minor
              << "." << (int)compiled.patch
              << std::endl;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#if 0
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                        SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif

    std::cerr << "SDL video driver: " << SDL_GetCurrentVideoDriver() << std::endl;

    SDL_Window * window = SDL_CreateWindow("imgui + sdl2 + opengl",
                                           SDL_WINDOWPOS_CENTERED,
                                           SDL_WINDOWPOS_CENTERED,
                                           1280,
                                           720,
                                           SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (window) {
        std::cerr << "SDL_CreateWindow done" << std::endl;
    } else {
        std::cerr << "SDL_CreateWindow failed: [" << SDL_GetError() << "]" << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);

    int major, minor;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);

    std::cerr << "Requested OpenGL version: " << major << "." << minor << std::endl;

    if (gl_context) {
        std::cerr << "SDL_GL_CreateContext done" << std::endl;
    } else {
        std::cerr << "SDL_GL_CreateContext failed: [" << SDL_GetError() << "]" << std::endl;
        return -1;
    }

    if (SDL_GL_MakeCurrent(window, gl_context) != 0) {
        std::cerr << "SDL_GL_MakeCurrent failed: [" << SDL_GetError() << "]" << std::endl;
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_GL_SetSwapInterval(1); // enable vsync

    GLenum glew_status = glewInit();
    if (glew_status == GLEW_OK) {
        std::cerr << "glewInit done" << std::endl;
    } else {
        std::cerr << "glewInit failed: [" << glewGetErrorString(glew_status) << "]" << std::endl;
        return -1;
    }

    GLenum gl_error = glGetError();

    if (gl_error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after glewInit: [" << gl_error << "]" << std::endl;
        return -1;
    }

    if (!glGetString(GL_VENDOR)) {
        std::cerr << "No valid OpenGL context" << std::endl;
        return -1;
    }

    const GLubyte * version = glGetString(GL_VERSION);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (version) {
        std::cerr << "OpenGL version: [" << version << "]" << std::endl;
    } else {
        std::cerr << "OpenGL version not available" << std::endl;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO & io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);

    if (ImGui_ImplOpenGL3_Init("#version 330")) {
        bool show_demo_window = true;
        bool show_another_window = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        float counter_value = 0.0f;

        // Main Loop
        bool done = false;

        while (!done) {
            /** poll + handle events */
            SDL_Event event;

            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type == SDL_QUIT)
                    done = true;

                if ((event.type == SDL_WINDOWEVENT)
                    && (event.window.event == SDL_WINDOWEVENT_CLOSE)
                    && (event.window.windowID == SDL_GetWindowID(window)))
                {
                    done = true;
                }
            }

            int w, h;
            SDL_GetWindowSize(window, &w, &h);
            glViewport(0, 0, w, h);
            glClearColor(clear_color.x * clear_color.w,
                         clear_color.y * clear_color.w,
                         clear_color.z * clear_color.w,
                         clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);

            // draw dear imgui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(io.DisplaySize);
            ImGui::Begin("Background", nullptr,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                         | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus
                         | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoDecoration);
            ImGui::End();

            // 1. big demo window
            if (show_demo_window)
                ImGui::ShowDemoWindow(&show_demo_window);

            // 2. show window that we create ourselves
            {
                static int counter = 0;

                ImGui::Begin("Hello, world!");
                ImGui::Text("This is totes useful text...");
                ImGui::Checkbox("demo window", &show_demo_window);
                ImGui::Checkbox("second window", &show_another_window);

                ImGui::SliderFloat("float", &counter_value, 0.0f, 1.0f);
                ImGui::ColorEdit3("clear color", (float*)&clear_color);

                if (ImGui::Button("Button"))
                    ++counter;
                ImGui::SameLine();
                ImGui::Text("counter = %d", counter);

                ImGui::Text("appl average %.3f ms/frame (%.1f fps)",
                            1000.0f / io.Framerate, io.Framerate);

                ImGui::End();
            }

            // 3. another window
            if (show_another_window) {
                ImGui::Begin("another window", &show_another_window);

                ImGui::Text("hello from second window");
                if (ImGui::Button("close me"))
                    show_another_window = false;

                ImGui::End();
            }

            // rendering
            ImGui::Render();

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            SDL_GL_SwapWindow(window);
        }

        std::cerr << "Shutting down.." << std::endl;

        ImGui_ImplOpenGL3_Shutdown();
    } else {
        std::cerr << "Failed to initialize imgui opengl3 backend, shutting down.." << std::endl;
    }

    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cerr << "Shutdown complete, goodbye..." << std::endl;

    return 0;

#ifdef NOPE2
    if (!glfwInit()) {
        std::cerr << "glfwInit failed!" << std::endl;
        return -1;
    }

    std::cout << "GLFW version: " << glfwGetVersionString() << std::endl;

    // configure glfw
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // this fails in nix-on-wsl
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    // maybe:
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // may need explicit hints
    //glfwWindowHint(GLFW_RED_BITS, 8)
    //glfwWindowHint(GLFW_GREEN_BITS, 8)
    //glfwWindowHint(GLFW_BLUE_BITS, 8)
    //glfwWindowHint(GLFW_ALPHA_BITS, 8)
    //glfwWindowHint(GLFW_DEPTH_BITS, 24)
    //glfwWindowHint(GLFW_STENCIL_BITS, 8)
    //glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE)
    //
    // try
    //   $ glxinfo
    // to get framebuffer info

    // Not working because:
    //   eglinfo
    // looks in
    //   /run/opengl-driver/share/glvnd/egl_vendor.d
    //   /etc/glvnd/egl_vendor.d
    //   /usr/share/glvnd/egl_vendor.d
    //
    // these tell EGL which vendor libraries to load (mesa | nvidia)

    // create window
    GLFWwindow * window = glfwCreateWindow(800, 600, "simple opengl triangle", NULL, NULL);

    if (!window) {
        // getting error couldn't find framebuffer configuration matching requested
        // opengl context.  used by GLX (opengl extension to x11).
        // defines things like: color depth, double buffering, stencil bits etc.


        const char * descr = nullptr;
        int code = glfwGetError(&descr);
        std::cerr << "glfwCreateWindow failed :code " << code << " :msg "
                  << (descr ? descr : "?unknown")
                  << std::endl;
    }
#endif

#ifdef NOPE

    if (SDL_Init(SDL_INIT_VIDEO
                 | SDL_INIT_TIMER
                 | SDL_INIT_GAMECONTROLLER) != 0)
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    // OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    // SDL window
    SDL_Window * window
        = SDL_CreateWindow("xo imgui ex1",
                           SDL_WINDOWPOS_CENTERED,
                           SDL_WINDOWPOS_CENTERED,
                           800 /*width ?*/,
                           600 /*height ?*/,
                           SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    SDL_GLContext gl_context = SDL_GLContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);  // enable vsync.. (wut!?)

    // imgui

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO & io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
#endif

#ifdef NOPE
    // this gets: Failed to initialize OpenGL loader!
    ImGui_ImplOpenGL3_Init("#version 330");
#endif
}

/* imgui_ex1.cpp */
