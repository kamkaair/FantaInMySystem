#pragma once
#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glm/glm.hpp>      // Include glm

class GameObject : public kgfw::Object {
public:
    GameObject(const char* const functionName);
    ~GameObject();

    void setPosition(const glm::vec3& position);
    void setRotationZ(float angleInRadians);
    void setRotationY(float angleInRadians);
    void setRotationX(float angleInRadians);
    void setRotation(const glm::vec3& eulerAngles);
    void setScaling(const glm::vec3& scale);

    void setLookAt(const glm::vec3& to, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));
    void setViewMatrix(const glm::vec3& to, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));

    const glm::vec3& getPosition() const;
    float getRotationZ() const;
    float getRotationY() const;
    float getRotationX() const;
    glm::vec3 getRotation() const;
    const glm::vec3& getScaling() const;

    glm::mat4 GameObject::getLookAt() const;
    glm::mat4 getModelMatrix() const;
    glm::mat4 GameObject::getViewMatrix() const;

    glm::mat4 ZoomFunc(float fov, float width, float height, float bimbim, float bambam);

private:
    // Model position, rotation and scale
    glm::vec3 m_position;           // Store position of plane here
    glm::vec3 m_scale;              // Store scale of plane here
    glm::vec3 m_rotation;
    glm::vec3 m_axis;
	glm::mat4 m_oritentation;
    glm::mat4 m_viewMatrix;
    float m_angleZInRadians;        // Store Z-angle of plane here
    float m_angleYInRadians;        // Store X-angle of plane here
    float m_angleXInRadians;        // Store X-angle of plane here
    float m_angleTotalInRadians;
};
