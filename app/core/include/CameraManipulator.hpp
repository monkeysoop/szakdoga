#pragma once

#include "Camera.hpp"

#include <glm/glm.hpp>
#include <SDL2/SDL.h>



namespace szakdoga::core {
    class CameraManipulator {
    public:
        CameraManipulator();

        ~CameraManipulator();

        void SetCamera(Camera* _pCamera);
        void Update(float _deltaTime);

        inline void SetSpeed(float _speed) { m_speed = _speed; }
        inline float GetSpeed() const noexcept { return m_speed; }

        void KeyboardDown(const SDL_KeyboardEvent& key);
        void KeyboardUp(const SDL_KeyboardEvent& key);
        void MouseMove(const SDL_MouseMotionEvent& mouse);
        void MouseWheel(const SDL_MouseWheelEvent& wheel);

    private:
        Camera* m_pCamera = nullptr;

        // The u spherical coordinate of the spherical coordinate pair (u,v) denoting the
        // current viewing direction from the view position m_eye.
        float m_u = 0.0f;

        // The v spherical coordinate of the spherical coordinate pair (u,v) denoting the
        // current viewing direction from the view position m_eye.
        float m_v = 0.0f;

        // The distance of the look at point from the camera.
        float m_distance = 0.0f;

        // The center of model sphere.
        glm::vec3 m_center = glm::vec3(0.0f);

        // The traversal speed of the camera
        float m_speed = 16.0f;

        // Traveling indicator to different directions.
        float m_goForward = 0.0f;
        float m_goRight = 0.0f;
        float m_goUp = 0.0f;

        bool m_forward_pressed = false;
        bool m_backward_pressed = false;
        bool m_left_pressed = false;
        bool m_right_pressed = false;
        bool m_up_pressed = false;
        bool m_down_pressed = false;
    };
} // namespace szakdoga::core