#pragma once
#include "kgfw/Object.h"
#include "shader.h"
#include "GBuffer.h"
#include "utils.h"
#include "UI.h"
#include <random>

struct SSAO_Settings {
	bool useSSAO = true;
	int kernelSize = 64;
	float radius = 0.5f;
	float bias = 0.025f;
	float occlusionStrength = 10.0f;
	bool clampedMidTones = false;

	bool dirty = false;
};

struct SSR_Settings {
	bool useSSR = true;
	bool useTA = true;
	bool useRayScattering = true;
	bool useBinaryRefinement = false;
	int maxSteps = 5;
	float thickness = 0.00014;
	float rayDirMin = 0.001;

	bool dirty = false;
};

class ScreenSpace : public kgfw::Object {
public:
	ScreenSpace(GBuffer* gbuffer, int inWidth, int inHeight);
	~ScreenSpace();

	void constructSSAO();
	void constructSSR();
	void deconstructSSAO();
	void deconstructSSR();
	void setupSSAO();

	void constructDeferredRendering();
	void constructForwardRendering();

	void renderSSAO(Camera* m_camera, UI* m_uiDraw, Mesh* m_meshRender, int width, int height, int samples);
	void renderSSR(Camera* m_camera, Mesh* m_meshRender, UI* m_uiDraw);
	void renderSSR_TA(Camera* m_camera, Mesh* m_meshRender);
	void renderCompositeShader(Mesh* m_meshRender, Camera* m_camera, HDRI* m_HDRI, UI* m_uiDraw);
	void recreateColorBuffer();

	void resetTA_SSR();

	void updateSSAOUniforms();
	void updateSSRUniforms();
	SSAO_Settings& getSSAO_Settings() { return m_ssaoSettings; }
	SSR_Settings& getSSR_Settings() { return m_ssrSettings; }

	std::vector<glm::vec3> createSampleKernel(std::uniform_real_distribution<GLfloat> randomFloats, std::default_random_engine generator);
	GLuint createNoiseTexture(std::uniform_real_distribution<GLfloat> randomFloats, std::default_random_engine generator);

	// Create textures
	GLuint createSsaoColorBuffer();
	GLuint createSsaoColorBufferBlur();
	GLuint createSsrSceneColorBuffer();
	GLuint createSsrSceneColorBufferBlur();

	// Create FBOs
	GLuint createSsaoFBO();
	GLuint createSsaoBlurFBO();
	GLuint createSsrFBO();
	GLuint createSsrBlurFBO();
	void createSSR_HistoryFramebuffer();
	void createTemporalBuffers();

	// Screen space gets
	GLuint getSsaoColorBuffer() { return ssaoColorBuffer; }
	GLuint getSsaoBlurColorBuffer() { return ssaoColorBufferBlur; }

	GLuint getSsrColorBuffer() { return ssrColorBuffer; }
	GLuint getSsrBlurColorBuffer() { return ssrColorBufferBlur; }

	// Shaders
	Shader* getSsaoShader() { return m_SSAO; }
	Shader* getSsrShader() { return m_SSR; }
	Shader* getSsrTaShader() { return m_SSR_TA; }

	// Temp accumulation
	GLuint getSSRHistoryRead() const { return ssrTemporalBuffer[1 - ssrHistoryIndex]; }
	GLuint getSSRHistoryWriteFBO() const { return ssrHistoryFBO[ssrHistoryIndex]; }
	void swapSSRHistory() { ssrHistoryIndex = 1 - ssrHistoryIndex; }

private:
	SSAO_Settings m_ssaoSettings;
	SSR_Settings m_ssrSettings;

	int width = 640, height = 480, ssrHistoryIndex = 0;
	int frameIndex = 0;

	Shader* m_SSAO = 0;
	Shader* m_SSR_TA = 0;
	Shader* m_blurSSAO = 0;
	Shader* m_SSR = 0;
	Shader* m_blurSSR = 0;
	GBuffer* m_GBuffer;

	GLuint ssaoFBO = 0, ssaoBlurFBO = 0, ssrFBO = 0, 
		ssrHistoryFBO[2], ssrBlurFBO = 0;

	GLuint ssaoColorBuffer = 0, ssaoColorBufferBlur = 0, noiseTexture = 0, 
		ssrColorBuffer = 0, ssrTemporalBuffer[2], ssrColorBufferBlur = 0;

	// For the temp accumulation of ssr
	GLuint prevDepthTex = 0, prevNormalTex = 0;
	GLuint prevDepthFBO = 0, prevNormalFBO = 0;
	glm::mat4 prevView, prevProj;
	bool firstFrame = true;

	std::vector<glm::vec3> ssaoKernel;
};