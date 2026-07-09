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
		radius -= (float)yoffset;
		if (radius < 1.0f) {
			radius = 1.0f;
		}

		//Max radius
		else if (radius > 20.0f) {
			radius = 20.0f;
		}
	}
}

void Inputs::inputMouse(GLFWwindow* window, double xposIn, double yposIn)
{
	if (mouseEnabled == false) {
		float xpos = static_cast<float>(xposIn);
		float ypos = static_cast<float>(yposIn);

		if (firstMouse) {
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
		lastX = xpos;
		lastY = ypos;

		//Mouse sensitivity
		float sensitivity = 0.1f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		
		m_cam->getCameraFront().x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		m_cam->getCameraFront().y = sin(glm::radians(pitch));
		m_cam->getCameraFront().z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		m_cam->getCameraFront() = glm::normalize(m_cam->getCameraFront());
	}
}

void Inputs::orbitCursorLeft(GLFWwindow* window, double xposIn, double yposIn)
{
	if (mouseLeftEnabled) {
		float dx = float(xposIn - xPos);
		float dy = float(yposIn - yPos);

		if (firstMouse) {
			xPos = dx;
			yPos = dy;
			firstMouse = false;
		}

		theta -= dx * orbitSens;
		phi += dy * orbitSens;
		xPos = xposIn;
		yPos = yposIn;

		// Clamp phi to avoid gimbal lock
		phi = glm::clamp(phi, epsilon, glm::pi<float>() - epsilon);
	}
}

void Inputs::orbitCursorRight(GLFWwindow* window, double xposIn, double yposIn)
{
	if (mouseRightEnabled) {
		float dx = float(xposIn - xPos);
		float dy = float(yposIn - yPos);

		if (firstMouse) {
			xPos = dx;
			yPos = dy;
			firstMouse = false;
		}

		xPos = xposIn;
		yPos = yposIn;

		glm::vec3 cameraRight = glm::normalize(glm::cross(m_cam->getCameraFront(), m_cam->getCameraUp()));
		glm::vec3 cameraUpAdjust = glm::normalize(glm::cross(cameraRight, m_cam->getCameraFront()));

		// Added times cameraRight for the X-axis to make the cameraFocus transforming according to camera's view
		m_cam->getCameraFocus() += cameraRight * dx * focusSens;
		m_cam->getCameraFocus() += cameraUpAdjust * -dy * focusSens;
	}
}

glm::vec3 Inputs::calculateCameraPosition() {
	float x = radius * sinf(phi) * cosf(theta) + m_cam->getCameraFocus().x;
	float y = radius * cosf(phi) + m_cam->getCameraFocus().y;
	float z = radius * sinf(phi) * sinf(theta) + m_cam->getCameraFocus().z;

	return glm::vec3(x, y, z);
}

void Inputs::setCameraFocusPoint(glm::vec3& focusPoint) {
	const float pi = 3.14159265359;

	yaw = 0;
	pitch = 0;

	// Direction of the focusPoint
	glm::vec3 dirVec = normalize(focusPoint - m_cam->getCameraPos());

	float newYaw = atan2(dirVec.z, dirVec.x);
	float newPitch = asin(dirVec.y);

	float yawDeg = newYaw * (180.0f / pi);
	float pitchDeg = newPitch * (180.0f / pi);

	yaw = yawDeg;
	pitch = pitchDeg;

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
		if (!m_cam->getIsMovementOrbit()) {
			togglePressedMovement = true;
			m_cam->getIsMovementOrbit() = !m_cam->getIsMovementOrbit();
			setCameraFocusPoint(m_cam->getCameraFocus());
		}
		else {
			togglePressedMovement = true;
			m_cam->getIsMovementOrbit() = !m_cam->getIsMovementOrbit();
			getCamera()->setFOV(40.0f);
		}
	}
	if ((glfwGetKey(window, GLFW_KEY_V)) == GLFW_RELEASE && !ImGui::GetIO().WantTextInput)
		togglePressedMovement = false;
}

void Inputs::mousePosUpdate(GLFWwindow* window) {
	glfwGetCursorPos(window, &xPos, &yPos);
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
	m_cam->getIsMovementOrbit() ? movementFreeMode(window, deltaTime) : movementOrbitMode(window);
}