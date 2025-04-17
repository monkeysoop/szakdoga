#pragma once

#include "Camera.hpp"
#include "CameraManipulator.hpp"
#include "Framebuffer.hpp"
#include "CompShader.hpp"
#include "Skybox.hpp"
#include "Texture2D.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <filesystem>



namespace szakdoga::core {
    enum class RenderModeType : unsigned {
        NORMAL = 0,
        ITERATION_COUNT = 1,
        SDF_CALL_COUNT = 2,
        DEBUG = 3,
        DEBUG_ITERATION_COUNT = 4,
        DEBUG_DEPTH = 5,
    };

    enum class SphereTracingType : unsigned {
        NAIVE = 0,
        RELAXED = 1,
        ENHANCED = 2,
    };

    enum class TracingType : unsigned {
        NAIVE = 0,
        RELAXED = 1,
        ENHANCED = 2,
        CONE = 3,
    };

    enum class SDFSceneType : unsigned {
        NEWTONS_CRADLE = 0,
        CAR = 1,
        TEMPLE = 2,
        PRIMITIVES = 3,
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
        TracingType m_tracing_type;
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

        float m_cone_trace_intermediate_epsilon;

        float m_cone_trace_relaxed_step_multiplier;
        float m_cone_trace_enhanced_step_multiplier;
        float m_cone_trace_enhanced_max_step_factor;

        float m_cone_trace_final_relaxed_step_multiplier;
        float m_cone_trace_final_enhanced_step_multiplier;
        float m_cone_trace_final_enhanced_max_step_factor;

        unsigned m_benchmark_baseline_iteration_count;
        unsigned m_benchmark_min_iteration_count;
        unsigned m_benchmark_max_iteration_count;
        unsigned m_benchmark_iteration_count_spacing;
        unsigned m_benchmark_performance_number_of_runs;

        float m_shadow_penumbra;
        float m_shadow_intensity;
        unsigned m_shadow_max_iteration_count;
        float m_ao_multiplier_attenuation;
        float m_ao_step_size;
        float m_ao_strength;
        unsigned m_ao_max_iteration_count;
        float m_ambient_strength;
        float m_reflection_attenuation;

        unsigned u_max_number_of_reflections;

    private:
        void SphereTraceRender();
        void ConeTraceRender();
        void PrecomputeCones();
        unsigned DivideAndRoundUp(unsigned number, unsigned divisor);
        void Benchmark();
        void BenchmarkSingleSphere(const std::filesystem::path& base_path, SDFSceneType sdf_scene, RenderModeType render_mode, SphereTracingType sphere_tracing_type);
        void BenchmarkSingleCone(const std::filesystem::path& base_path, SDFSceneType sdf_scene, RenderModeType render_mode, SphereTracingType cone_tracing_type, SphereTracingType cone_final_tracing_type);
        void WriteTimeTaken(const std::filesystem::path& path, double ms_taken_chrono, double ms_taken_gl);
        void RecompileSphereTracer(SDFSceneType sdf_scene, RenderModeType render_mode, SphereTracingType sphere_tracing_type);
        void RecompileConeTracer(SDFSceneType sdf_scene, SphereTracingType cone_tracing_type);
        void RecompileConeFinalTracer(SDFSceneType sdf_scene, RenderModeType render_mode, SphereTracingType cone_final_tracing_type);
    };
} // namespace szakdoga::core