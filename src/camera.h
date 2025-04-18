#pragma once
#include "gameobject.h"     // Include base class
#include <glm/gtx/transform.hpp>	// glm transform functions.

class Camera : public GameObject {
public:
    Camera(float fov, float aspect, float near, float far)
        : GameObject(__FUNCTION__), m_fov(fov), m_aspect(aspect), m_near(near), m_far(far) {
        updateProjectionMatrix();
    }

    ~Camera() {

    }

    const glm::mat4& getProjectionMatrix() const { return m_projection; }

    // FOV
    void setFOV(float fov) {
        m_fov = fov;
        updateProjectionMatrix();  // Update the projection matrix when FOV changes
    }

    void setAspectRatio(float width, float height)
    {
        m_width = width;
        m_height = height;

        m_aspect = static_cast<float>(width) / static_cast<float>(height);
        updateProjectionMatrix();
    }

    float getFOV() const { return m_fov; }

private:
    glm::mat4 m_projection;

    float m_fov;
    float m_width, m_height, m_near, m_far;
    float m_aspect;

    void updateProjectionMatrix() {
        m_projection = glm::perspective(glm::radians(m_fov), m_width / m_height, m_near, m_far);
    }
};
