#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glad/gl.h>		// Include glad
#include <glm/glm.hpp>      // Include glm
#include "glm/gtx/string_cast.hpp" // Include for printing mats and vecs
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <fstream>
#include <filesystem>
#include <stb_image.h>
#include <iostream>

#include "shader.h"
#include "material.h"
#include "mesh.h"
#include "utils.h"
#include "textureLoading.h"
#include "HDRI.h"

struct SettingsMaterial {
	glm::vec3 diffuseColor = glm::vec3(1.0f);  // Default white color
	float roughness = 0.5f;                    // Default roughness
	float metallic = 0.0f;                     // Default metallic value

	GLuint diffuseTexture = 0;					// OpenGL texture ID
	GLuint metallicTexture = 0;
	GLuint roughnessTexture = 0;
	GLuint normalTexture = 0;

	bool useDiffuseTexture = true;				// Whether to use a texture or a value
	bool useMetallicTexture = true;
	bool useRoughnessTexture = true;
};

class UI : public kgfw::Object
{
public:
	UI(Shader* shader,
		Shader* backImage,
		TextureLoading* texLoad,
		HDRI* hdri);
	~UI();

	void ImGuiStyleSetup();
	void ImGuiDraw();

	// Gets and Sets
	int getBackgroundMode() const { return backgroundMode; }

	void toggleMeshRotation() { meshRotationEnabled = !meshRotationEnabled; }
	bool boolMeshRotation() const { return meshRotationEnabled; }

	void toggleDoOnce() { doOnce = !doOnce; }
	bool boolDoOnce() const { return doOnce; }

	void toggleWireframe() { wireFrame = !wireFrame; }
	void setImGuiAlpha(float alpha) { ImGuiAlpha = alpha; }

	bool getRenderMode() { return deferredRendering; }

	// Add '&' to get the REFERENCE!!!
	std::vector<Mesh*>& getMeshes() { return m_meshes; }
	std::vector<glm::vec3>& getPointLightPos() { return pointLightPos; }
	std::vector<glm::vec3>& getPointLightColor() { return pointLightColor; }

private:
	SettingsMaterial m_settingsMaterial;

	Shader* m_shader; // Shader reference
	Shader* m_backImage;
	std::vector<Mesh*> m_meshes; // Mesh reference
	TextureLoading* m_texLoading;
	HDRI* m_HDRI;

	std::vector<glm::vec3> pointLightPos; // Reference to point light positions
	std::vector<glm::vec3> pointLightColor; // Reference to point light colors
	std::vector<std::string> texTypes = { "Diffuse", "Metallic", "Roughness", "Normal" };

	float lampStrength = 0.0f;
	float HdrContrast = 2.2f;
	float HdrExposure = 1.0f;
	float ImGuiAlpha = 0.3f;
	float HueChange = 1.0f;
	float backExposure = 1.0f;
	float backContrast = 2.2f;

	float totalScale = 0.0f;

	const char* backgroundOptions[3] = { "HDRI","Texture","Solid Color" };
	int backgroundMode = 0;
	bool meshRotationEnabled = true;
	bool doOnce = true;
	bool wireFrame = false;
	bool scaleLock = false;
	bool meshHide = false;
	bool deferredRendering = false;

	glm::vec3 originalScale = { 1.0f, 1.0f, 1.0f };
	GLfloat backgroundColor[4] = { 0.2, 0.2, 0.2, 1.0 };
};