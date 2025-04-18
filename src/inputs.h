#pragma once
#include "gameobject.h"     // Include base class
#include "UI.h"

class Inputs : public kgfw::Object {
public:
	Inputs(UI* ui, Camera* camera);
	~Inputs();

	void inputFocus(GLFWwindow* window);
	void inputHide(GLFWwindow* window);
	void inputScroll(GLFWwindow* window, double xoffset, double yoffset, float fov);
	void inputMouse(GLFWwindow* window, double xposIn, double yposIn);
	void inputMovement(GLFWwindow* window, float deltaTime);
	void inputMouseMovement(GLFWwindow* window, double xposIn, double yposIn);

	glm::vec3 getCameraPos() { return cameraPos; }
	glm::vec3 getCameraFront() { return cameraFront; }
	glm::vec3 getCameraUp() { return cameraUp; }
	bool getImGuiVisibility() { return isHidden; }

	void setCameraPos(glm::vec3 newPos) { cameraPos = newPos; }

private:
	// Camera movement
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.5f, 1.0f), cameraFront = glm::vec3(0.0f, 0.0f, -1.0f), cameraUp = glm::vec3(0.0f, 1.0f, 0.0f), cameraFocus = glm::vec3(0.0f, 0.0f, 0.0f);
	// Is mouse active?
	bool firstMouse = true;
	bool mouseEnabled = false;
	float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
	float pitch = 0.0f;
	float lastX = 800.0f / 2.0;
	float lastY = 600.0 / 2.0;

	float radius = 10.0f;

	bool togglePressed = false;
	bool togglePressedHide = false;
	bool isHidden = false;

	UI* m_uiDraw;
	Camera* m_camera;
};