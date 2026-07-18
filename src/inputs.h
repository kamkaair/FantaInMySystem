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
	void movementControls(GLFWwindow* window, float deltaTime);

	void mousePosUpdate(GLFWwindow* window);
	glm::vec3 calculateCameraPosition();
	void setDeltaTime(float deltaTime) { m_deltaTime = deltaTime; }

	Camera* getCamera() { return m_cam; }

	bool getImGuiVisibility() { return isHidden; }
	bool getMovementMode() { return m_cam->getIsMovementFree(); }
	bool getMouseEnabled() { return mouseEnabled; }

	void setMouseLeftEnabled(bool inState) { mouseLeftEnabled = inState; }
	void setMouseRightEnabled(bool inState) { mouseRightEnabled = inState; }
	void setMovementMode(bool newMode) { m_cam->getIsMovementFree() = newMode; }
	void toggleMovementMode(GLFWwindow* window);

	void setCameraFocusPoint(glm::vec3& focusPoint);
	void setCameraPos(glm::vec3& newPos) { m_cam->cameraPos = newPos; }
	void setCameraFront(glm::vec3& newFront) { m_cam->cameraFront = newFront; }

	void setImGuiInteractability(GLFWwindow* window, int cursorMode, float ImGuiAlpha, float orbitSens, float focusSens, bool WindowInteract);

private:
	void movementFreeMode(GLFWwindow* window, float deltaTime);
	void movementOrbitMode(GLFWwindow* window);
	void updateCameraVectors();

	// Is mouse active?
	bool firstMouse = true;
	bool mouseEnabled = false, mouseLeftEnabled = false, mouseRightEnabled = false;
	bool togglePressed = false, togglePressedHide = false, togglePressedMovement = false, isHidden = false;

	float m_orbitSens = 0.0005f, m_focusSens = 0.004f, m_deltaTime = 0.0f;
	const float epsilon = 0.01f;

	UI* m_uiDraw;
	Camera* m_cam;
};