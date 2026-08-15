#pragma once
#include "kgfw/Object.h"
#include "shader.h"
#include "GBuffer.h"
#include "utils.h"
#include "UI.h"
#include <random>

struct SSAO_SETTINGS {
	int kernelSize = 64;
	float radius = 0.5f;
	float bias = 0.025f;
	float occlusionStrength = 10.0f;

	bool clampedMidTones = false;	
	bool useSSAO = true;
	bool dirty = false;
};

struct SSR_SETTINGS {
	int maxSteps = 5;
	float thickness = 0.00014;
	float rayDirMin = 0.001;

	bool useSSR = true;
	bool useTA = true;
	bool useRayScattering = true;
	bool useBinaryRefinement = false;
	bool dirty = false;
};

struct BLOOM_SETTINGS {
	int amount = 10;
	int distance = 5;
	bool useBloom = true;
	bool dirty = false;
};

class ScreenSpace : public kgfw::Object {
public:
	ScreenSpace(GBuffer* gbuffer);
	~ScreenSpace();

	void constructSSAO();
	void constructSSR();
	void deconstructSSAO();
	void deconstructSSR();
	void setupSSAO();

	void constructDeferredRendering();
	void constructForwardRendering();

	void renderSSAO(Camera* m_camera, Mesh* m_meshRender, int samples);
	void renderSSR(Camera* m_camera, Mesh* m_meshRender);
	void renderSSR_TA(Camera* m_camera, Mesh* m_meshRender);
	void renderBloom(Mesh* m_meshRender, const GLuint& inLightTexture);
	void renderCompositeShader(Mesh* m_meshRender);

	void recreateColorBuffer();
	void clearPingPongBuffer();
	void resetTA_SSR();

	void updateSSAOUniforms();
	void updateSSRUniforms();
	void updateBloomUniforms();
	SSAO_SETTINGS& getSSAO_Settings() { return m_ssaoSettings; }
	SSR_SETTINGS& getSSR_Settings() { return m_ssrSettings; }
	BLOOM_SETTINGS& getBloom_Settings() { return m_bloomSettings; }
	GLuint ScreenSpace::getSSR_History() { return m_ssrSettings.useTA ? getSSRHistoryRead() : ssrColorBuffer; }

	std::vector<glm::vec3> createSampleKernel(std::uniform_real_distribution<GLfloat> randomFloats, std::default_random_engine generator);
	GLuint createNoiseTexture(std::uniform_real_distribution<GLfloat> randomFloats, std::default_random_engine generator);
	void createPingPongBuffer();

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
	SSAO_SETTINGS m_ssaoSettings;
	SSR_SETTINGS m_ssrSettings;
	BLOOM_SETTINGS m_bloomSettings;
	
	Shader* m_SSAO = 0;
	Shader* m_SSR_TA = 0;
	Shader* m_blurSSAO = 0;
	Shader* m_SSR = 0;
	Shader* m_blurSSR = 0;
	Shader* m_gaussianBlur = 0;
	GBuffer* m_GBuffer;	

	// SSAO
	GLuint ssaoColorBuffer = 0, ssaoColorBufferBlur = 0, noiseTexture = 0;
	GLuint ssaoFBO = 0, ssaoBlurFBO = 0;
	std::vector<glm::vec3> ssaoKernel;

	// SSR
	GLuint ssrColorBuffer = 0, ssrTemporalBuffer[2], ssrColorBufferBlur = 0;
	GLuint ssrFBO = 0, ssrHistoryFBO[2], ssrBlurFBO = 0;

	// Temporary accumulation for SSR
	GLuint prevDepthTex = 0, prevNormalTex = 0;
	GLuint prevDepthFBO = 0, prevNormalFBO = 0;
	glm::mat4 prevView, prevProj;
	bool firstFrame = true;
	int ssrHistoryIndex = 0;
	int frameIndex = 0;

	// Bloom
	GLuint pingpongFBO[2];
	GLuint pingpongBuffer[2];
	bool horizontal = true;
};