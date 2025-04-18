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
	
	//Update camera position and LookAt direction
	m_camera->setPosition(cameraPos);
	//m_camera->setLookAt(cameraPos + cameraFront); // redundant

	//Update view matrix
	m_camera->setViewMatrix(cameraPos + cameraFront);
}

void Inputs::inputMouseMovement(GLFWwindow* window, double xposIn, double yposIn) {
	if (!ImGui::GetIO().WantTextInput) {
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			float camX = static_cast<float>(sin(glfwGetTime()) * radius);
			float camY = static_cast<float>(tan(glfwGetTime()) * radius);
			float camZ = static_cast<float>(cos(glfwGetTime()) * radius);

			//float camZ = sin(glfwGetTime() * radius);
			//glm::mat4 view = glm::lookAt(glm::vec3(camX, 0.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

			//Update camera position and LookAt direction
			//m_camera->setPosition(cameraPos);
			m_camera->setPosition(glm::vec3(camX, 0.0f, camZ));

			//Update view matrix
			//m_camera->setViewMatrix(cameraPos + cameraFront);
			m_camera->setViewMatrix(glm::vec3(0.0f, 0.0f, 0.0f));

			printf("BIBINKI");
		}
	}
}