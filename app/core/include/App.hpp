#pragma once

#include "Camera.hpp"
#include "CameraManipulator.hpp"
#include "Framebuffer.hpp"
#include "CompShader.hpp"
#include "Skybox.hpp"
#include "Texture2D.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <filesystem>



namespace szakdoga::core {
    enum class RenderModeType : unsigned {
        NORMAL = 0,
        ITERATION_COUNT = 1,
        DEPTH = 2,
    };

    enum class SphereTracingType : unsigned {
        NAIVE = 0,
        RELAXED = 1,
        ENHANCED = 2,
        CONE = 3,
    };

    enum class SDFSceneType : unsigned {
        NEWTONS_CRADLE = 0,
        CAR = 1,
        TEMPLE = 2,
    };

    class App {
    public:
        App(unsigned width, unsigned height);
        ~App();

        void Update(float elapsed_time_in_seconds, float delta_time_in_seconds);
        void Render();
        void RenderImGui();

        void KeyboardDown(const SDL_KeyboardEvent& key);
        void KeyboardUp(const SDL_KeyboardEvent& key);
        void MouseMove(const SDL_MouseMotionEvent& mouse);
        void MouseWheel(const SDL_MouseWheelEvent& wheel);
        void Resize(unsigned width, unsigned height);

    private:
        unsigned m_width;
        unsigned m_height;

        Camera m_camera;
        CameraManipulator m_camera_manipulator;

        Framebuffer m_framebuffer;

        SDFSceneType m_sdf_scene;
        RenderModeType m_render_mode;
        SphereTracingType m_sphere_tracing_type;
        SphereTracingType m_cone_trace_sphere_tracing_type;
        SphereTracingType m_cone_trace_final_sphere_tracing_type;

        CompShader m_sphere_trace_shader;
        CompShader m_cone_trace_shader;
        CompShader m_cone_trace_final_shader;
        CompShader m_cone_trace_precompute_shader;

        Texture2D m_cone_trace_distance_iteration_texture_1;
        Texture2D m_cone_trace_distance_iteration_texture_2;

        unsigned m_initial_cone_size;
        Texture2D m_cone_trace_precomputed_texture;

        Skybox m_skybox;

        float m_time_in_seconds;
        float m_epsilon;
        float m_max_distance;
        unsigned m_max_iteration_count;

        float m_relaxed_step_multiplier;
        float m_enhanced_step_multiplier;
        float m_enhanced_max_step_factor;

        float m_cone_trace_relaxed_step_multiplier;
        float m_cone_trace_enhanced_step_multiplier;
        float m_cone_trace_enhanced_max_step_factor;

        float m_cone_trace_final_relaxed_step_multiplier;
        float m_cone_trace_final_enhanced_step_multiplier;
        float m_cone_trace_final_enhanced_max_step_factor;

    private:
        void SphereTraceRender();
        void ConeTraceRender();
        void PrecomputeCones();
        unsigned DivideAndRoundUp(unsigned number, unsigned divisor);
        void Benchmark();
        void BenchmarkSingle(const std::filesystem::path& base_path, SDFSceneType sdf_scene, RenderModeType render_mode, SphereTracingType sphere_tracing_type);
    };
} // namespace szakdoga::core