#include "inputs.h"
#include <GLFW/glfw3.h>				// Include glfw for windows

Inputs::Inputs(UI* ui, Camera* camera) : m_uiDraw(ui), m_cam(camera), Object(__FUNCTION__) {}

Inputs::~Inputs(){}

void Inputs::setImGuiInteractability(GLFWwindow* window, int cursorMode, float ImGuiAlpha, float orbitSensIn, float focusSensIn, bool WindowInteract) {
	glfwSetInputMode(window, GLFW_CURSOR, cursorMode);
	m_uiDraw->setImGuiAlpha(ImGuiAlpha);
	orbitSens = orbitSensIn; focusSens = focusSensIn;
	m_uiDraw->setWindowInteract(WindowInteract);
}

void Inputs::inputFocus(GLFWwindow* window) {
	// Toggle mouse cursor with 'E'
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !togglePressed && !ImGui::GetIO().WantTextInput) {
		togglePressed = true;
		mouseEnabled = !mouseEnabled; // Set mouseEnabled to what it's not

		if (mouseEnabled) { setImGuiInteractability(window, GLFW_CURSOR_NORMAL, 0.9f, 0.0f, 0.0f, false); }
		else {
			firstMouse = true;
			setImGuiInteractability(window, GLFW_CURSOR_DISABLED, 0.3f, 0.0005f, 0.004f, true);
		}
	}
	// Using GLFW_RELEASE to avoid detecting multiple presses for the toggle
	else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE && !ImGui::GetIO().WantTextInput)
		togglePressed = false;
}

void Inputs::inputHide(GLFWwindow* window) {
	// Flip-flop for setting ImGui window hidden
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !togglePressedHide && !ImGui::GetIO().WantTextInput) {
		togglePressedHide = true;
		//m_uiDraw->toggleIsHidden();
		isHidden = !isHidden;
	}
	else if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE && !ImGui::GetIO().WantTextInput)
		togglePressedHide = false;
	//return isHidden;
}

void Inputs::inputScrollFOV(GLFWwindow* window, double xoffset, double yoffset, float fov)
{
	if (mouseEnabled == false) {

		//Min FOV
		fov -= (float)yoffset;
		if (fov < 1.0f)
			fov = 1.0f;

		//Max FOV
		else if (fov > 45.0f)
			fov = 45.0f;

		// Apply the new FOV to the camera
		m_cam->setFOV(fov);
	}
}

void Inputs::inputScrollRadius(GLFWwindow* window, double xoffset, double yoffset, float fov)
{
	if (mouseEnabled == false) {

		//Min radius
		m_cam->getRadius() -= (float)yoffset;
		if (m_cam->getRadius() < 1.0f) {
			m_cam->getRadius() = 1.0f;
		}

		//Max radius
		else if (m_cam->getRadius() > 20.0f) {
			m_cam->getRadius() = 20.0f;
		}
	}
}

void Inputs::inputMouse(GLFWwindow* window, double xposIn, double yposIn)
{
	if (mouseEnabled == false) {
		float xpos = static_cast<float>(xposIn);
		float ypos = static_cast<float>(yposIn);

		if (firstMouse) {
			
			m_cam->getLastX() = xpos;
			m_cam->getLastY() = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - m_cam->getLastX();
		float yoffset = m_cam->getLastY() - ypos; // reversed since y-coordinates go from bottom to top
		m_cam->getLastX() = xpos;
		m_cam->getLastY() = ypos;

		//Mouse sensitivity
		float sensitivity = 0.05f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		m_cam->getYaw() += xoffset;
		m_cam->getPitch() += yoffset;
		
		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (m_cam->getPitch() > 89.0f)
			m_cam->getPitch() = 89.0f;
		if (m_cam->getPitch() < -89.0f)
			m_cam->getPitch() = -89.0f;

		
		m_cam->getCameraFront().x = cos(glm::radians(m_cam->getYaw())) * cos(glm::radians(m_cam->getPitch()));
		m_cam->getCameraFront().y = sin(glm::radians(m_cam->getPitch()));
		m_cam->getCameraFront().z = sin(glm::radians(m_cam->getYaw())) * cos(glm::radians(m_cam->getPitch()));
		m_cam->getCameraFront() = glm::normalize(m_cam->getCameraFront());
	}
}

void Inputs::orbitCursorLeft(GLFWwindow* window, double xposIn, double yposIn)
{
	if (mouseLeftEnabled) {
		float dx = float(xposIn - m_cam->getPosX());
		float dy = float(yposIn - m_cam->getPosY());

		if (firstMouse) {
			m_cam->getPosX() = dx;
			m_cam->getPosY() = dy;
			firstMouse = false;
		}
		
		m_cam->getTheta() -= dx * orbitSens;
		m_cam->getPhi() += dy * orbitSens;
		m_cam->getPosX() = xposIn;
		m_cam->getPosY() = yposIn;

		// Clamp phi to avoid gimbal lock
		m_cam->getPhi() = glm::clamp(m_cam->getPhi(), epsilon, glm::pi<float>() - epsilon);
	}
}

void Inputs::orbitCursorRight(GLFWwindow* window, double xposIn, double yposIn)
{
	if (mouseRightEnabled) {
		float dx = float(xposIn - m_cam->getPosX());
		float dy = float(yposIn - m_cam->getPosY());

		if (firstMouse) {
			m_cam->getPosX() = dx;
			m_cam->getPosY() = dy;
			firstMouse = false;
		}

		m_cam->getPosX() = xposIn;
		m_cam->getPosY() = yposIn;

		glm::vec3 cameraRight = glm::normalize(glm::cross(m_cam->getCameraFront(), m_cam->getCameraUp()));
		glm::vec3 cameraUpAdjust = glm::normalize(glm::cross(cameraRight, m_cam->getCameraFront()));

		// Added times cameraRight for the X-axis to make the cameraFocus transforming according to camera's view
		m_cam->getCameraFocus() += cameraRight * dx * focusSens;
		m_cam->getCameraFocus() += cameraUpAdjust * -dy * focusSens;
	}
}

glm::vec3 Inputs::calculateCameraPosition() {
	float x = m_cam->getRadius() * sinf(m_cam->getPhi()) * cosf(m_cam->getTheta()) + m_cam->getCameraFocus().x;
	float y = m_cam->getRadius() * cosf(m_cam->getPhi()) + m_cam->getCameraFocus().y;
	float z = m_cam->getRadius() * sinf(m_cam->getPhi()) * sinf(m_cam->getTheta()) + m_cam->getCameraFocus().z;

	return glm::vec3(x, y, z);
}

void Inputs::setCameraFocusPoint(glm::vec3& focusPoint) {
	const float pi = 3.14159265359;

	m_cam->getYaw() = 0;
	m_cam->getPitch() = 0;

	// Direction of the focusPoint
	glm::vec3 dirVec = normalize(focusPoint - m_cam->getCameraPos());

	float newYaw = atan2(dirVec.z, dirVec.x);
	float newPitch = asin(dirVec.y);

	float yawDeg = newYaw * (180.0f / pi);
	float pitchDeg = newPitch * (180.0f / pi);

	m_cam->getYaw() = yawDeg;
	m_cam->getPitch() = pitchDeg;

	// Set the cameraFront's direction
	m_cam->getCameraFront() = glm::normalize(m_cam->getCameraFocus() - m_cam->getCameraPos());
	firstMouse = true; // Set firstMouse true to combat the camera snapping
}

void Inputs::updateCameraVectors() {
	m_cam->getCameraPos() = calculateCameraPosition(); // Orbiting camera position
	m_cam->getCameraFront() = glm::normalize(m_cam->getCameraFocus() - m_cam->getCameraPos());
}

void Inputs::toggleMovementMode(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && !togglePressedMovement && !ImGui::GetIO().WantTextInput) {
		if (!m_cam->getIsMovementFree()) {
			togglePressedMovement = true;
			m_cam->getIsMovementFree() = !m_cam->getIsMovementFree();
			setCameraFocusPoint(m_cam->getCameraFocus());
		}
		else {
			togglePressedMovement = true;
			m_cam->getIsMovementFree() = !m_cam->getIsMovementFree();
			getCamera()->setFOV(40.0f);
		}
	}
	if ((glfwGetKey(window, GLFW_KEY_V)) == GLFW_RELEASE && !ImGui::GetIO().WantTextInput)
		togglePressedMovement = false;
}

void Inputs::mousePosUpdate(GLFWwindow* window) {
	glfwGetCursorPos(window, &m_cam->getPosX(), &m_cam->getPosY());
}

void Inputs::movementFreeMode(GLFWwindow* window, float deltaTime) {
	// Free camera movement
	if (!ImGui::GetIO().WantTextInput)
	{
		// "Sprint"
		float cameraSpeed = 1.0 * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			cameraSpeed = 2.5 * deltaTime;
		
		// WASD movement controls, descend/ascend with space and l-control
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			m_cam->getCameraPos() += cameraSpeed * m_cam->getCameraFront();
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			m_cam->getCameraPos() -= cameraSpeed * m_cam->getCameraFront();
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			m_cam->getCameraPos() -= glm::normalize(glm::cross(m_cam->getCameraFront(), m_cam->getCameraUp())) * cameraSpeed;
		else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			m_cam->getCameraPos() += glm::normalize(glm::cross(m_cam->getCameraFront(), m_cam->getCameraUp())) * cameraSpeed;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			m_cam->getCameraPos() += cameraSpeed * m_cam->getCameraUp();
		else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			m_cam->getCameraPos() -= cameraSpeed * m_cam->getCameraUp();
	}
	
	//Update camera position and LookAt direction
	m_cam->setPosition(m_cam->getCameraPos());
	m_cam->setLookAt(m_cam->getCameraPos() + m_cam->getCameraFront()); // redundant

	//Update view matrix
	m_cam->setViewMatrix(m_cam->getCameraPos() + m_cam->getCameraFront());
}

void Inputs::movementOrbitMode(GLFWwindow* window) {
	// Orbiting controls
	updateCameraVectors();

	glm::vec3 camPos = calculateCameraPosition();
	m_cam->setPosition(camPos);
	m_cam->setLookAt(m_cam->getCameraPos() + m_cam->getCameraFront()); // redundant
	m_cam->setViewMatrix(m_cam->getCameraPos() + m_cam->getCameraFront());

	mousePosUpdate(window);
}

void Inputs::movementControls(GLFWwindow* window, float deltaTime) {
	m_cam->getIsMovementFree() ? movementFreeMode(window, deltaTime) : movementOrbitMode(window);
}