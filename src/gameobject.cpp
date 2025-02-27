#include "gameobject.h"
#include <glm/gtx/transform.hpp>	// glm transform functions.

GameObject::GameObject(const char* const functionName)
    : Object(functionName) {
    // Initialize transform
	m_oritentation = glm::mat4(1.0f);
    setPosition(glm::vec3(0.0));
    setRotationZ(0.0f);
    setRotationY(0.0f);
    setRotationX(0.0f);
    setRotation(glm::vec3(0.0));
    setScaling(glm::vec3(1.0f));
}

GameObject::~GameObject() {
}

// SETS
void GameObject::setPosition(const glm::vec3& position) {
    m_position = position;
}

void GameObject::setRotationZ(float angleInRadians) {
    m_angleZInRadians = angleInRadians;
}

void GameObject::setRotationY(float angleInRadians) {
    m_angleYInRadians = angleInRadians;
}

void GameObject::setRotationX(float angleInRadians) {
    m_angleXInRadians = angleInRadians;
}

void GameObject::setRotation(const glm::vec3& eulerAngles) {
    m_angleXInRadians = eulerAngles.x;
    m_angleYInRadians = eulerAngles.y;
    m_angleZInRadians = eulerAngles.z;
}

void GameObject::setScaling(const glm::vec3& scale) {
    m_scale = scale;
}

void GameObject::setViewMatrix(const glm::vec3& to, const glm::vec3& up) {
    m_viewMatrix = glm::lookAt(m_position, to, up);
}

void GameObject::setLookAt(const glm::vec3& to, const glm::vec3& up) {
    m_oritentation = glm::inverse(glm::lookAtRH(getPosition(), to, up));
}


// GETS
const glm::vec3& GameObject::getPosition() const {
    return m_position;
}

float GameObject::getRotationZ() const {
    return m_angleZInRadians;
}

float GameObject::getRotationY() const {
    return m_angleYInRadians;
}

float GameObject::getRotationX() const {
    return m_angleXInRadians;
}

glm::vec3 GameObject::getRotation() const {
    return glm::vec3(m_angleXInRadians, m_angleYInRadians, m_angleZInRadians);
}

const glm::vec3& GameObject::getScaling() const {
    return m_scale;
}

glm::mat4 GameObject::getLookAt() const {
    return m_oritentation;
}

glm::mat4 GameObject::getViewMatrix() const {
    return m_viewMatrix;
}

glm::mat4 GameObject::getModelMatrix() const {
    return glm::translate(glm::mat4(1.0f), m_position)
			* m_oritentation
            * glm::rotate(m_angleZInRadians, glm::vec3(0.0f, 0.0f, 1.0f))
            * glm::rotate(m_angleYInRadians, glm::vec3(1.0f, 0.0f, 0.0f))
            * glm::rotate(m_angleXInRadians, glm::vec3(0.0f, 1.0f, 0.0f))
            * glm::scale(glm::mat4(1.0f), m_scale);
}