#include "CameraManipulator.hpp"

#include "Camera.hpp"

#include <SDL2/SDL.h>

CameraManipulator::CameraManipulator() {}

CameraManipulator::~CameraManipulator() {}

void CameraManipulator::SetCamera(Camera* _pCamera) {
    m_pCamera = _pCamera;

    if (!m_pCamera)
        return;

    // Set the initial spherical coordinates.
    m_center = m_pCamera->GetAt();
    glm::vec3 ToAim = m_center - m_pCamera->GetEye();

    m_distance = glm::length(ToAim);

    m_u = atan2f(ToAim.z, ToAim.x);
    m_v = acosf(ToAim.y / m_distance);
}

void CameraManipulator::Update(float _deltaTime) {
    if (!m_pCamera)
        return;

    // Frissitjuk a kamerát a Model paraméterek alapján.

    // Az új nézési irányt a gömbi koordináták alapján számoljuk ki.
    glm::vec3 lookDirection(cosf(m_u) * sinf(m_v), cosf(m_v), sinf(m_u) * sinf(m_v));
    // Az új kamera pozíciót a nézési irány és a távolság alapján számoljuk ki.
    glm::vec3 eye = m_center - m_distance * lookDirection;

    // Az új felfelé irány a világ felfelével legyen azonos.
    glm::vec3 up = m_pCamera->GetWorldUp();

    // Az új jobbra irányt a nézési irány és a felfelé irány keresztszorzatából számoljuk ki.
    glm::vec3 right = glm::normalize(glm::cross(lookDirection, up));

    // Az új előre irányt a felfelé és jobbra irányok keresztszorzatából számoljuk ki.
    glm::vec3 forward = glm::cross(up, right);

    // Az új elmozdulásat a kamera mozgás irányának és sebességének a segítségével számoljuk ki.
    glm::vec3 deltaPosition = (m_goForward * forward + m_goRight * right + m_goUp * up) * m_speed * _deltaTime;

    // Az új kamera pozíciót és nézési cél pozíciót beállítjuk.
    eye += deltaPosition;
    m_center += deltaPosition;

    // Frissítjük a kamerát az új pozícióval és nézési iránnyal.
    m_pCamera->SetView(eye, m_center, m_pCamera->GetWorldUp());
}

void CameraManipulator::KeyboardDown(const SDL_KeyboardEvent& key) {
    switch (key.keysym.sym) {
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            if (key.repeat == 0)
                m_speed /= 4.0f;
            break;
        case SDLK_w:
            m_forward_pressed = true;
            m_goForward = 1;
            break;
        case SDLK_s:
            m_backward_pressed = true;
            m_goForward = -1;
            break;
        case SDLK_a:
            m_left_pressed = true;
            m_goRight = -1;
            break;
        case SDLK_d:
            m_right_pressed = true;
            m_goRight = 1;
            break;
        case SDLK_e:
            m_up_pressed = true;
            m_goUp = 1;
            break;
        case SDLK_q:
            m_down_pressed = true;
            m_goUp = -1;
            break;
    }
}

void CameraManipulator::KeyboardUp(const SDL_KeyboardEvent& key) {
    switch (key.keysym.sym) {
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            m_speed *= 4.0f;
            break;
        case SDLK_w:
            m_forward_pressed = false;
            m_goForward = m_backward_pressed ? -1 : 0;
            break;
        case SDLK_s:
            m_backward_pressed = false;
            m_goForward = m_forward_pressed ? 1 : 0;
            break;
        case SDLK_a:
            m_left_pressed = false;
            m_goRight = m_right_pressed ? 1 : 0;
            break;
        case SDLK_d:
            m_right_pressed = false;
            m_goRight = m_left_pressed ? -1 : 0;
            break;
        case SDLK_q:
            m_down_pressed = false;
            m_goUp = m_up_pressed ? 1 : 0;
            break;
        case SDLK_e:
            m_up_pressed = false;
            m_goUp = m_down_pressed ? -1 : 0;
            break;
    }
}

void CameraManipulator::MouseMove(const SDL_MouseMotionEvent& mouse) {
    if (mouse.state & SDL_BUTTON_LMASK) {
        float du = mouse.xrel / 100.0f;
        float dv = mouse.yrel / 100.0f;

        m_u += du;
        m_v = glm::clamp<float>(m_v + dv, 0.1f, 3.1f);
    }
    if (mouse.state & SDL_BUTTON_RMASK) {
        float dDistance = mouse.yrel / 100.0f;
        m_distance += dDistance;
    }
}

void CameraManipulator::MouseWheel(const SDL_MouseWheelEvent& wheel) {
    float dDistance = static_cast<float>(wheel.y) * m_speed / -100.0f;
    m_distance += dDistance;
}