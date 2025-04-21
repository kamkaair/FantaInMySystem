#include "inputs.h"

Inputs::Inputs(UI* ui, Camera* camera) : m_uiDraw(ui), m_camera(camera), Object(__FUNCTION__) {}

Inputs::~Inputs(){}

void Inputs::inputFocus(GLFWwindow* window) {
	// Toggle mouse cursor with 'E'
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !togglePressed && !ImGui::GetIO().WantTextInput) {
		togglePressed = true;
		mouseEnabled = !mouseEnabled; // Set mouseEnabled to what it's not

		if (mouseEnabled) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			m_uiDraw->setImGuiAlpha(0.9f);
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			m_uiDraw->setImGuiAlpha(0.3f);
			firstMouse = true;
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

void Inputs::inputScroll(GLFWwindow* window, double xoffset, double yoffset, float fov)
{
	if (mouseEnabled == false) {

		//Min FOV
		fov -= (float)yoffset;
		if (fov < 1.0f)
			fov = 1.0f;

		//Max FOV
		if (fov > 45.0f)
			fov = 45.0f;

		// Apply the new FOV to the camera
		m_camera->setFOV(fov);

		//std::cout << "New FOV: " << fov << std::endl;
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

		// update the front vector
		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(front);

		//std::cout << glm::to_string(cameraFront) << std::endl;
	}
}

void Inputs::inputMouseCursorLeft(GLFWwindow* window, double xposIn, double yposIn)
{
	if (mouseLeftEnabled) {
		float sensitivity = 0.00001f;
		float dx = float(xposIn - xPos);
		float dy = float(yposIn - yPos);

		theta -= dx * sensitivity;
		phi += dy * sensitivity;

		// Clamp phi to avoid gimbal lock
		const float epsilon = 0.01f;
		phi = glm::clamp(phi, epsilon, glm::pi<float>() - epsilon);

		lastX = xposIn;
		lastY = yposIn;
	}
}

void Inputs::inputMouseCursorRight(GLFWwindow* window, double xposIn, double yposIn)
{
	if (mouseRightEnabled) {
		float sensitivity = 0.00001f;
		float dx = float(xposIn - xPos);
		float dy = float(yposIn - yPos);

		yaw += dx;
		pitch += dy;

		// update the front vector
		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(front);

		// TODO: cameraFocus still a bit tacky with X-axis movement. Fix
		glm::vec3 up(0.0f, 1.0f, 0.0f);
		glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraFront));
		glm::vec3 cameraUpAdjust = glm::normalize(glm::cross(cameraFront, cameraRight));

		cameraFocus += dx * sensitivity * cameraRight;
		cameraFocus += dy * sensitivity * cameraUpAdjust;

		//cameraFocus += glm::vec3(dx * sensitivity, 0.0f, 0.0f);
		//cameraFocus -= glm::vec3(0.0f, dy * sensitivity, 0.0f);
		std::cout << glm::to_string(cameraFocus) << std::endl;

		lastX = xposIn;
		lastY = yposIn;
	}
}

glm::vec3 Inputs::calculateCameraPosition() {
	float x = radius * sinf(phi) * cosf(theta);
	float y = radius * cosf(phi);
	float z = radius * sinf(phi) * sinf(theta);
	return glm::vec3(x, y, z);
}

void Inputs::mousePosUpdate(GLFWwindow* window) {
	glfwGetCursorPos(window, &xPos, &yPos);
}

void Inputs::inputMovement(GLFWwindow* window, float deltaTime) {
	if (!ImGui::GetIO().WantTextInput)
	{
		// Camera movement
		// "Sprint"
		float cameraSpeed = 1.0 * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			cameraSpeed = 2.5 * deltaTime;

		// Movement controls
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			cameraPos += cameraSpeed * cameraFront;
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			cameraPos -= cameraSpeed * cameraFront;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			cameraPos += cameraSpeed * cameraUp;
		else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			cameraPos -= cameraSpeed * cameraUp;
	}

	std::cout << glm::to_string(cameraFront) << std::endl;
	
	//Update camera position and LookAt direction
	m_camera->setPosition(cameraPos);
	m_camera->setLookAt(cameraPos + cameraFront); // redundant

	//Update view matrix
	m_camera->setViewMatrix(cameraPos + cameraFront);
}