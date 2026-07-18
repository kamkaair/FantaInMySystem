#include "inputs.h"
#include <GLFW/glfw3.h>				// Include glfw for windows

Inputs::Inputs(UI* ui, Camera* camera) : m_uiDraw(ui), m_cam(camera), Object(__FUNCTION__) {}

Inputs::~Inputs(){}

void Inputs::setImGuiInteractability(GLFWwindow* window, int cursorMode, float ImGuiAlpha, float orbitSensIn, float focusSensIn, bool WindowInteract) {
	glfwSetInputMode(window, GLFW_CURSOR, cursorMode);
	m_uiDraw->setImGuiAlpha(ImGuiAlpha);
	m_orbitSens = orbitSensIn; m_focusSens = focusSensIn;
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
		m_cam->radius -= (float)yoffset;
		if (m_cam->radius < 1.0f) {
			m_cam->radius = 1.0f;
		}

		//Max radius
		else if (m_cam->radius > 20.0f) {
			m_cam->radius = 20.0f;
		}
	}
}

void Inputs::inputMouse(GLFWwindow* window, double xposIn, double yposIn)
{
	if (mouseEnabled == false) {
		if (firstMouse) {
			
			m_cam->lastX = xposIn;
			m_cam->lastY = yposIn;
			firstMouse = false;
		}

		float xpos = static_cast<float>(xposIn);
		float ypos = static_cast<float>(yposIn);

		float xoffset = xpos - m_cam->lastX;
		float yoffset = m_cam->lastY - ypos; // reversed since y-coordinates go from bottom to top
		m_cam->lastX = xpos;
		m_cam->lastY = ypos;

		//Mouse sensitivity
		float sensitivity = 4.00f * m_deltaTime;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		m_cam->yaw += xoffset;
		m_cam->pitch += yoffset;
		
		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (m_cam->pitch > 89.0f)
			m_cam->pitch = 89.0f;
		if (m_cam->pitch < -89.0f)
			m_cam->pitch = -89.0f;
	
		m_cam->cameraFront.x = cos(glm::radians(m_cam->yaw)) * cos(glm::radians(m_cam->pitch));
		m_cam->cameraFront.y = sin(glm::radians(m_cam->pitch));
		m_cam->cameraFront.z = sin(glm::radians(m_cam->yaw)) * cos(glm::radians(m_cam->pitch));
		m_cam->cameraFront = glm::normalize(m_cam->cameraFront);
	}
}

void Inputs::orbitCursorLeft(GLFWwindow* window, double xposIn, double yposIn) {
	if (mouseLeftEnabled) {
		if (firstMouse) {
			m_cam->xPos = xposIn;
			m_cam->yPos = yposIn;
			firstMouse = false;
		}

		float dx = float(xposIn - m_cam->xPos);
		float dy = float(yposIn - m_cam->yPos);
		
		float orbitSens = 50 * m_orbitSens * m_deltaTime;

		m_cam->theta -= dx * orbitSens;
		m_cam->phi += dy * orbitSens;
		m_cam->xPos = xposIn;
		m_cam->yPos = yposIn;

		// Clamp phi to avoid gimbal lock
		m_cam->phi = glm::clamp(m_cam->phi, epsilon, glm::pi<float>() - epsilon);
	}
}

void Inputs::orbitCursorRight(GLFWwindow* window, double xposIn, double yposIn) {
	if (mouseRightEnabled) {
		if (firstMouse) {
			m_cam->xPos = xposIn;
			m_cam->yPos = yposIn;
			firstMouse = false;
		}

		float dx = float(xposIn - m_cam->xPos);
		float dy = float(yposIn - m_cam->yPos);

		m_cam->xPos = xposIn;
		m_cam->yPos = yposIn;

		glm::vec3 cameraRight = glm::normalize(glm::cross(m_cam->cameraFront, m_cam->cameraUp));
		glm::vec3 cameraUpAdjust = glm::normalize(glm::cross(cameraRight, m_cam->cameraFront));

		// Added times cameraRight for the X-axis to make the cameraFocus transforming according to camera's view
		float focusSens = 50 * m_focusSens * m_deltaTime;
		m_cam->cameraFocus += cameraRight * dx * focusSens;
		m_cam->cameraFocus += cameraUpAdjust * -dy * focusSens;
	}
}

glm::vec3 Inputs::calculateCameraPosition() {
	float x = m_cam->radius * sinf(m_cam->phi) * cosf(m_cam->theta) + m_cam->cameraFocus.x;
	float y = m_cam->radius * cosf(m_cam->phi) + m_cam->cameraFocus.y;
	float z = m_cam->radius * sinf(m_cam->phi) * sinf(m_cam->theta) + m_cam->cameraFocus.z;

	return glm::vec3(x, y, z);
}

void Inputs::setCameraFocusPoint(glm::vec3& focusPoint) {
	const float pi = 3.14159265359;

	m_cam->yaw = 0;
	m_cam->pitch = 0;

	// Direction of the focusPoint
	glm::vec3 dirVec = normalize(focusPoint - m_cam->cameraPos);

	float newYaw = atan2(dirVec.z, dirVec.x);
	float newPitch = asin(dirVec.y);

	float yawDeg = newYaw * (180.0f / pi);
	float pitchDeg = newPitch * (180.0f / pi);

	m_cam->yaw = yawDeg;
	m_cam->pitch = pitchDeg;

	// Set the cameraFront's direction
	m_cam->cameraFront = glm::normalize(m_cam->cameraFocus - m_cam->cameraPos);
	firstMouse = true; // Set firstMouse true to combat the camera snapping
}

void Inputs::updateCameraVectors() {
	m_cam->cameraPos = calculateCameraPosition(); // Orbiting camera position
	m_cam->cameraFront = glm::normalize(m_cam->cameraFocus - m_cam->cameraPos);
}

void Inputs::toggleMovementMode(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && !togglePressedMovement && !ImGui::GetIO().WantTextInput) {
		if (!m_cam->getIsMovementFree()) {
			togglePressedMovement = true;
			m_cam->getIsMovementFree() = !m_cam->getIsMovementFree();
			setCameraFocusPoint(m_cam->cameraFocus);
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
	glfwGetCursorPos(window, &m_cam->xPos, &m_cam->yPos);
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
			m_cam->cameraPos += cameraSpeed * m_cam->cameraFront;
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			m_cam->cameraPos -= cameraSpeed * m_cam->cameraFront;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			m_cam->cameraPos -= glm::normalize(glm::cross(m_cam->cameraFront, m_cam->cameraUp)) * cameraSpeed;
		else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			m_cam->cameraPos += glm::normalize(glm::cross(m_cam->cameraFront, m_cam->cameraUp)) * cameraSpeed;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			m_cam->cameraPos += cameraSpeed * m_cam->cameraUp;
		else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			m_cam->cameraPos -= cameraSpeed * m_cam->cameraUp;
	}
	
	//Update camera position and LookAt direction
	m_cam->setPosition(m_cam->cameraPos);
	m_cam->setLookAt(m_cam->cameraPos + m_cam->cameraFront); // redundant

	//Update view matrix
	m_cam->setViewMatrix(m_cam->cameraPos + m_cam->cameraFront);
}

void Inputs::movementOrbitMode(GLFWwindow* window) {
	// Orbiting controls
	updateCameraVectors();

	m_cam->setPosition(m_cam->cameraPos);
	m_cam->setLookAt(m_cam->cameraPos + m_cam->cameraFront); // redundant
	m_cam->setViewMatrix(m_cam->cameraPos + m_cam->cameraFront);

	mousePosUpdate(window);
}

void Inputs::movementControls(GLFWwindow* window, float deltaTime) {
	m_cam->getIsMovementFree() ? movementFreeMode(window, deltaTime) : movementOrbitMode(window);
}