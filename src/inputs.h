#pragma once
#include "gameobject.h"     // Include base class
#include "UI.h"
#include "camera.h"
#include <iostream> // KILL

class Inputs : public kgfw::Object {
public:
	Inputs(UI* ui, Camera* camera);
	~Inputs();

	void inputFocus(GLFWwindow* window);
	void inputHide(GLFWwindow* window);
	void inputScrollFOV(GLFWwindow* window, double xoffset, double yoffset, float fov);
	void inputScrollRadius(GLFWwindow* window, double xoffset, double yoffset, float fov);
	void inputMouse(GLFWwindow* window, double xposIn, double yposIn);

	void orbitCursorLeft(GLFWwindow* window, double xposIn, double yposIn);
	void orbitCursorRight(GLFWwindow* window, double xposIn, double yposIn);
	void movementControls(GLFWwindow* window, float deltaTime); // private pls??
	void movementFreeMode(GLFWwindow* window, float deltaTime); // private pls
	void movementOrbitMode(GLFWwindow* window); // private pls

	void mousePosUpdate(GLFWwindow* window);

	void updateCameraVectors();
	glm::vec3 calculateCameraPosition();

	Camera* getCamera() { return m_cam; }

	bool getImGuiVisibility() { return isHidden; }
	bool getMovementMode() { return m_cam->getIsMovementOrbit(); }
	bool getMouseEnabled() { return mouseEnabled; }

	void setMouseLeftEnabled(bool inState) { mouseLeftEnabled = inState; }
	void setMouseRightEnabled(bool inState) { mouseRightEnabled = inState; }
	void setMovementMode(bool newMode) { m_cam->getIsMovementOrbit() = newMode; }
	void toggleMovementMode(GLFWwindow* window);

	void setCameraFocusPoint(glm::vec3& focusPoint);
	void setCameraPos(glm::vec3& newPos) { m_cam->getCameraPos() = newPos; }
	void setCameraFront(glm::vec3& newFront) { m_cam->getCameraFront() = newFront; }

	void setImGuiInteractability(GLFWwindow* window, int cursorMode, float ImGuiAlpha, float orbitSens, float focusSens, bool WindowInteract);

private:
	// Is mouse active?
	bool firstMouse = true;
	bool mouseEnabled = false, mouseLeftEnabled = false, mouseRightEnabled = false;
	bool togglePressed = false, togglePressedHide = false, togglePressedMovement = false, isHidden = false;

	float pitch = 0.0f, yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
	float lastX = 800.0f / 2.0, lastY = 600.0 / 2.0;
	float radius = 10.0f, theta = 0.0f, phi = 3.14159265359f / 4.0f, orbitSens = 0.0005f, focusSens = 0.004f;
	const float epsilon = 0.01f;

	double xPos = 0.0f, yPos = 0.0f;

	UI* m_uiDraw;
	Camera* m_cam;
};