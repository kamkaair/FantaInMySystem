#pragma once
#include "gameobject.h"     // Include base class
#include "UI.h"

class Inputs : public kgfw::Object {
public:
	Inputs(UI* ui, Camera* camera);
	~Inputs();

	void focus_callback(GLFWwindow* window);
	void hide_callback(GLFWwindow* window);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset, float fov);
	void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
	void movement(GLFWwindow* window, float deltaTime);

	glm::vec3 getCameraPos() { return cameraPos; }
	glm::vec3 getCameraFront() { return cameraFront; }
	glm::vec3 getCameraUp() { return cameraUp; }

	void setCameraPos(glm::vec3 newPos) { cameraPos = newPos; }

private:
	// Camera movement
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.5f, 1.0f), cameraFront = glm::vec3(0.0f, 0.0f, -1.0f), cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	// Is mouse active?
	bool firstMouse = true;
	bool mouseEnabled = false;
	float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
	float pitch = 0.0f;
	float lastX = 800.0f / 2.0;
	float lastY = 600.0 / 2.0;
	//float fov = 40.0f;

	bool togglePressed = false;
	bool togglePressedHide = false;
	bool isHidden = false;

	UI* m_uiDraw;
	Camera* m_camera;
};