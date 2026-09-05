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

	int getBackgroundMode() const { return backgroundMode; }

	bool getLightOrientation() { return lightOrientationOn; }

	template<typename T> void shaderSet(const char* uniform, T value) {
		m_GBuffer->getCurrentShader()->setUniform(uniform, value);
	}

	template<typename T> void shaderSetDual(const char* uniform, T value, Shader* forwardShader, Shader* deferredShader = m_GBuffer->getCurrentShader()) {
		forwardShader->bind();
		forwardShader->setUniform(uniform, value);
		if (m_GBuffer->getRenderMode() && deferredShader != 0) {
			deferredShader->bind();
			deferredShader->setUniform(uniform, value);
		}
	}

	void shaderBind() { m_GBuffer->getCurrentShader()->bind(); }

	void setWindowInteract(bool newBool) { windowDisabled = newBool; }
	ImGuiWindowFlags disableInteraction();

private:
	PostProcess pp_HDRI, pp_background, pp_model;
	void updatePostProcess() {
		pp_HDRI.name = "HDRI";
		pp_background.name = "Background";
		pp_model.name = "FinalColor";
	}

	void renderPostProcessSliders(PostProcess& pp, Shader* forwardShader, Shader* deferredShader);
	void renderMaterialOptions(SettingsMaterial& SetMat, static int currentItem[]);
	void renderMeshTreeNode(Model* model, std::uint16_t nameIndex);
	void changeMaterial(Mesh* mesh);
	template<typename T> void meshTransformationUI(T* meshes, glm::vec3 values[3], std::string name);

	Shader* m_backImage;

	HDRI* m_HDRI;
	GBuffer* m_GBuffer;
	ScreenSpace* m_SSAO;
	ResourceManager* m_resoManager;

	// File names
	std::string defaultFolderPath = "/textures";

	float ImGuiAlpha = 0.3f, totalScale = 0.0f;
	int backgroundMode = 0;
	
	const char* backgroundOptions[2] = { "HDRI","Texture" };

	bool meshRotationEnabled = false, doOnce = true, meshHide = false, windowDisabled = false, lightOrientationOn = true, useFolderFiltering = false;

	// UI settings
	bool m_useAutomaticTextures = false, m_wireFrame = false;

	const glm::vec3 originalScale = { 1.0f, 1.0f, 1.0f };
	GLfloat backgroundColor[4] = { 0.2, 0.2, 0.2, 1.0 };

	ImGuiWindowFlags flagWinDisabled = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoInputs;
	ImGuiWindowFlags flagWinEnabled = 0;
};