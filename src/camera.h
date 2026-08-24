#pragma once
#include "gameobject.h"     // Include base class
#include <glm/gtx/transform.hpp>	// glm transform functions.

class Camera : public GameObject {
public:
    Camera(float fov, float aspect, float near, float far)
        : GameObject(__FUNCTION__), m_fov(fov), m_aspect(aspect), m_near(near), m_far(far), m_width(1.0f), m_height(1.0f) {
        updateProjectionMatrix();
    }

    ~Camera() {}

    const glm::mat4& getProjectionMatrix() const { return m_projection; }
    //const glm::mat4& getViewMatrix() const { return m_view; }
    //const glm::mat4& getLookAt() const { return m_lookAt; }

    float getNear() { return m_near; }
    float getFar() { return m_far; }

    //void setLookAt(const glm::vec3& to, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f)) { m_lookAt = glm::inverse(glm::lookAtRH(getPosition(), to, up)); }
    //void setViewMatrix(const glm::vec3& to, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f)) { m_view = glm::lookAt(cameraPos, to, up); }

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
    
    bool& getIsMovementFree() { return m_freeMovement; }

    // Camera vectors
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.5f, 1.0f), cameraFront = glm::vec3(0.0f, 0.0f, -1.0f),
        cameraUp = glm::vec3(0.0f, 1.0f, 0.0f), cameraFocus = glm::vec3(0.0f, 0.0f, 0.0f);

    // Orbit values
    float radius = 10.0f, theta = 0.0f, phi = 3.14159265359f / 4.0f;

    float pitch = 0.0f, yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
    float lastX = 800.0f / 2.0, lastY = 600.0 / 2.0;
    double xPos = 0.0f, yPos = 0.0f;

private:
    glm::mat4 m_projection;
    //glm::mat4 m_projection, m_view, m_lookAt;

    float m_fov;
    float m_width, m_height, m_near, m_far;
    float m_aspect;

    bool m_freeMovement = false;

    void updateProjectionMatrix() {
        m_projection = glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
    }
};
