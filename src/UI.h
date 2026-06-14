#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <kgfw/Object.h>	// Include kgfw::Object to be used as a base class
#include <glad/gl.h>		// Include glad
#include <glm/glm.hpp>      // Include glm
#include "glm/gtx/string_cast.hpp" // Include for printing mats and vecs
#include <glm/gtc/type_ptr.hpp>

#include "mesh.h"
#include "textureLoading.h"
#include "HDRI.h"
#include "GBuffer.h"
#include "scene.h"

#include <vector>
#include <stb_image.h>
#include <initializer_list>

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

class SSAO;

class UI : public kgfw::Object
{
public:
	UI(Shader* backImage,
		TextureLoading* texLoad,
		HDRI* hdri,
		GBuffer* gbuffer,
		SSAO* ssao,
		Scene* scene);
	~UI();

	void ImGuiStyleSetup();
	void ImGuiDraw();

	// Gets and Sets
	void toggleMeshRotation() { meshRotationEnabled = !meshRotationEnabled; }
	bool boolMeshRotation() const { return meshRotationEnabled; }
	void toggleDoOnce() { doOnce = !doOnce; }
	bool boolDoOnce() const { return doOnce; }

	void toggleWireframe() { wireFrame = !wireFrame; }
	void setImGuiAlpha(float alpha) { ImGuiAlpha = alpha; }

	bool getRenderMode() { return deferredRendering; }
	int getBackgroundMode() const { return backgroundMode; }

	bool getLightOrientation() { return lightOrientationOn; }

	SettingsMaterial getSettingsMaterial() { return m_settingsMaterial; }

	void shaderSet(const char* uniform, float value) { m_GBuffer->getCurrentShader()->setUniform(uniform, value); }
	void shaderSet(const char* uniform, bool value) { m_GBuffer->getCurrentShader()->setUniform(uniform, value); }
	void shaderBind() { m_GBuffer->getCurrentShader()->bind(); }

	void updateFiles(std::vector<std::string>& fileNames, std::string location) {
		if (!fileNames.empty())
			fileNames.clear();

		for (const auto& file : m_texLoading->FileSystem((std::string(ASSET_DIR) + "/" + location))) {
			fileNames.push_back(file.c_str());
		}
	}

	void setWindowInteract(bool newBool) { windowDisabled = newBool; }
	ImGuiWindowFlags disableInteraction();

private:
	SettingsMaterial m_settingsMaterial;

	Shader* m_backImage;

	TextureLoading* m_texLoading;
	HDRI* m_HDRI;
	GBuffer* m_GBuffer;
	SSAO* m_SSAO;
	Scene* m_scene;

	// File names
	std::vector<std::string> m_saveFiles, meshFileNames, hdrFileNames;

	float HdrContrast = 2.2f, HdrExposure = 1.0f, ImGuiAlpha = 0.3f, 
		HueChange = 1.0f, backExposure = 1.0f, backContrast = 2.2f, totalScale = 0.0f;

	int backgroundMode = 0;
	
	const char* backgroundOptions[3] = { "HDRI","Texture","Solid Color" };
	std::vector<std::string> texTypes = { "Diffuse", "Metallic", "Roughness", "Normal" };

	bool meshRotationEnabled = true, doOnce = true, wireFrame = false, scaleLock = false, meshHide = false, 
		deferredRendering = false, windowDisabled = false, lightOrientationOn = true, useNormalTexture = true;

	glm::vec3 originalScale = { 1.0f, 1.0f, 1.0f };
	GLfloat backgroundColor[4] = { 0.2, 0.2, 0.2, 1.0 };

	ImGuiWindowFlags flagWinDisabled = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoInputs;
	ImGuiWindowFlags flagWinEnabled = 0;
};