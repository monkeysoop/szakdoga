#pragma once

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

namespace szakdoga::test {
    class Context {
    public:
        Context() {
            SDL_LogSetPriority(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR);

            if (SDL_Init(SDL_INIT_VIDEO) != 0) {
                throw std::runtime_error("Error during the SDL initialization: " + std::string(SDL_GetError()));
            }
        
            std::atexit(SDL_Quit);
        
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        
        
            m_window = SDL_CreateWindow(
                "something",
                SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,
                WIDTH,
                HEIGHT,
                SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
            );
            if (m_window == nullptr) {
                SDL_Quit();
                throw std::runtime_error("Error during the SDL window creation: " + std::string(SDL_GetError()));
            }
        
        
            m_gl_context = SDL_GL_CreateContext(m_window);
            if (m_gl_context == nullptr) {
                SDL_DestroyWindow(m_window);
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
                SDL_GL_DeleteContext(m_gl_context);
                SDL_DestroyWindow(m_window);
                SDL_Quit();
                throw std::runtime_error("Error during the initialization of glew: " + std::string(reinterpret_cast<const char*>(glewGetErrorString(glew_status))));
            }
        
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGui_ImplSDL2_InitForOpenGL(m_window, m_gl_context);
            ImGui_ImplOpenGL3_Init();
            ImGui::StyleColorsDark();
        }

        ~Context() {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplSDL2_Shutdown();
            ImGui::DestroyContext();
            
            if (m_gl_context != nullptr) {
                SDL_GL_DeleteContext(m_gl_context);
            }
            
            if (m_window != nullptr) {
                SDL_DestroyWindow(m_window);
            }
            
            SDL_Quit();
        }
    
        private:
            SDL_Window* m_window = nullptr;
            SDL_GLContext m_gl_context = nullptr;

    };
} // namespace szakdoga::test