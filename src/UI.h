#pragma once
#include "HDRI.h"
#include "GBuffer.h"
#include "scene.h"
#include "resourceManager.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <vector>
#include <stb_image.h>
#include <initializer_list>
#include <functional>
#include <iostream>

class ScreenSpace;

struct PostProcess {
	std::string name;
	float exposure = 1.0f;
	float contrast = 1.0f;
	glm::vec3 hue = { 1.0f, 1.0f, 1.0f };
};

class UI : public kgfw::Object
{
public:
	UI(Shader* backImage,
		HDRI* hdri,
		GBuffer* gbuffer,
		ScreenSpace* ssao,
		ResourceManager* resoManager);
	~UI();

	void ImGuiStyleSetup();
	void ImGuiDraw();

	void useAutomaticTextureFinding(const static std::uint8_t& currentItem);
	void useRegularModelLoading(const static std::uint8_t currentItem);

	// Gets and Sets
	void toggleMeshRotation() { meshRotationEnabled = !meshRotationEnabled; }
	bool boolMeshRotation() const { return meshRotationEnabled; }
	void toggleDoOnce() { doOnce = !doOnce; }
	bool boolDoOnce() const { return doOnce; }

	void toggleWireframe() { m_wireFrame = !m_wireFrame; }
	void setImGuiAlpha(float alpha) { ImGuiAlpha = alpha; }

	bool getRenderMode() { return deferredRendering; }
	int getBackgroundMode() const { return backgroundMode; }

	bool getLightOrientation() { return lightOrientationOn; }

	SettingsMaterial getSettingsMaterial() { return m_settingsCreateMat; }

	template<typename T> void shaderSet(const char* uniform, T value) {
		m_GBuffer->getCurrentShader()->setUniform(uniform, value);
	}

	template<typename T> void shaderSetDual(const char* uniform, T value, Shader* shader = m_GBuffer->getCurrentShader()) {
		shader->bind();
		shader->setUniform(uniform, value);
		if (deferredRendering) {
			m_GBuffer->getForwardShader()->bind();
			m_GBuffer->getForwardShader()->setUniform(uniform, value);
		}
	}

	void shaderBind() { m_GBuffer->getCurrentShader()->bind(); }

	void setWindowInteract(bool newBool) { windowDisabled = newBool; }
	ImGuiWindowFlags disableInteraction();

private:
	void updateAllFiles() { // TODO: probably for resource manager
		updateFiles(meshFileNames, "models/", [this](const std::string& path) { return m_resoManager->FileSystem(path); });
		updateFiles(m_materialFileNames, "textures/", [this](const std::string& path) { return m_resoManager->FileSystem(path); });
		updateFiles(m_saveFiles, "Saves/", [this](const std::string& path) { return m_resoManager->FileSystem(path); });
		updateFiles(hdrFileNames, "HDRI/", [this](const std::string& path) { return m_resoManager->FileSystem(path); });
		updateFiles(m_folderNames, "textures/", [this](const std::string& path) { return m_resoManager->FileSystemFolders(path); });
	}

	void updateFiles(std::vector<std::string>& fileNames, std::string location, std::function< std::vector<std::string>(const std::string path) > func) {
		if (!fileNames.empty())
			fileNames.clear();

		//fileNames = m_resoManager->FileSystem((std::string(ASSET_DIR) + "/" + location));
		fileNames = func(std::string(ASSET_DIR) + "/" + location);
	}

	PostProcess pp_HDRI, pp_background, pp_model;
	void updatePostProcess() {
		pp_HDRI.name = "HDRI";
		pp_background.name = "Background";
		pp_model.name = "FinalColor";
	}

	void renderPostProcessSliders(PostProcess& pp, Shader* inShader);
	void renderMaterialOptions(SettingsMaterial& SetMat, static int currentItem[]);
	void renderMeshTreeNode(Model* model, std::uint16_t nameIndex);

	SettingsMaterial m_settingsCreateMat, m_settingsEditMat;

	Shader* m_backImage;

	HDRI* m_HDRI;
	GBuffer* m_GBuffer;
	ScreenSpace* m_SSAO;
	ResourceManager* m_resoManager;

	// File names
	std::vector<std::string> m_saveFiles, meshFileNames, hdrFileNames, m_materialFileNames, m_folderNames;
	std::string defaultFolderPath = "/textures";

	float ImGuiAlpha = 0.3f, totalScale = 0.0f;
	int backgroundMode = 0;
	
	const char* backgroundOptions[2] = { "HDRI","Texture" };

	bool meshRotationEnabled = false, doOnce = true,  scaleLock = false, meshHide = false, 
		deferredRendering = false, windowDisabled = false, lightOrientationOn = true, useFolderFiltering = false;

	// UI settings
	bool m_useAutomaticTextures = false, m_wireFrame = false;

	const glm::vec3 originalScale = { 1.0f, 1.0f, 1.0f };
	GLfloat backgroundColor[4] = { 0.2, 0.2, 0.2, 1.0 };

	ImGuiWindowFlags flagWinDisabled = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoInputs;
	ImGuiWindowFlags flagWinEnabled = 0;
};