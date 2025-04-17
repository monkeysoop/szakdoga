#include "App.hpp"

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>



const unsigned WIDTH = 640;
const unsigned HEIGHT = 512;

int main() {
    SDL_LogSetPriority(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error("Error during the SDL initialization: " + std::string(SDL_GetError()));
    }

    std::atexit(SDL_Quit);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);


    SDL_Window* window = SDL_CreateWindow(
        "something",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH,
        HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (window == NULL) {
        SDL_Quit();
        throw std::runtime_error("Error during the SDL window creation: " + std::string(SDL_GetError()));
    }


    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == NULL) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw std::runtime_error("Error during the creation of the GL context: " + std::string(SDL_GetError()));
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_GL_SetSwapInterval(0);  // 0 - no vsync, 1 - vsync

    glewExperimental = GL_TRUE;
    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw std::runtime_error("Error during the initialization of glew: " + std::string(reinterpret_cast<const char*>(glewGetErrorString(glew_status))));
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init();
    ImGui::StyleColorsDark();

    using namespace szakdoga::core;
    App app{WIDTH, HEIGHT};


    bool running = true;
    bool show_imgui = false;

    Uint32 start_time = SDL_GetTicks();
    Uint32 last_time = SDL_GetTicks();
    unsigned n_frames = 0;

    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            ImGui_ImplSDL2_ProcessEvent(&ev);
            bool is_mouse_captured = ImGui::GetIO().WantCaptureMouse;
            bool is_keyboard_captured = ImGui::GetIO().WantCaptureKeyboard;

            switch (ev.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN:
                    if (ev.key.keysym.sym == SDLK_ESCAPE) {
                        running = false;
                    } else if (ev.key.keysym.sym == SDLK_i) {
                        show_imgui = !show_imgui;
                    } else if (!is_keyboard_captured) {
                        app.KeyboardDown(ev.key);
                    }
                    break;
                case SDL_KEYUP:
                    if (!is_keyboard_captured)
                        app.KeyboardUp(ev.key);
                    break;
                case SDL_MOUSEWHEEL:
                    if (!is_mouse_captured)
                        app.MouseWheel(ev.wheel);
                    break;
                case SDL_MOUSEMOTION:
                    if (!is_mouse_captured)
                        app.MouseMove(ev.motion);
                    break;
                case SDL_WINDOWEVENT:
                    if ((ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) || (ev.window.event == SDL_WINDOWEVENT_SHOWN)) {
                        int w, h;
                        SDL_GetWindowSize(window, &w, &h);
                        app.Resize(w, h);
                    }
                    break;
            }
        }

        n_frames++;
        Uint32 current_time = SDL_GetTicks();

        if ((current_time - start_time) > 1000) {
            std::cout << "FPS: " << n_frames << "\t\ttime per frame: " << (1000.0 / static_cast<float>(n_frames)) << " ms" << std::endl;
            n_frames = 0;
            start_time += 1000;
        }
        
        app.Update(static_cast<float>(current_time) / 1000.0f, static_cast<float>(current_time - last_time) / 1000.0f);
        
        last_time = current_time;
        
        app.Render();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();

        ImGui::NewFrame();
        if (show_imgui) {
            app.RenderImGui();
        }
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }


    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}