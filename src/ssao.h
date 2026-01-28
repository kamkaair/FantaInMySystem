#pragma once
#include "kgfw/Object.h"
#include "shader.h"
#include "GBuffer.h"
#include "utils.h"
#include "UI.h"
#include <random>

class SSAO : public kgfw::Object {
public:
	SSAO(GBuffer* gbuffer, int inWidth, int inHeight);
	~SSAO();

	void constructSSAO();
	void constructSSR();
	void deconstructSSAO();
	void deconstructSSR();
	void setupSSAO();
	void renderSSAO(Camera* m_camera, UI* m_uiDraw, Mesh* m_meshRender, int width, int height, int samples);
	void renderSSR(Camera* m_camera, Mesh* m_meshRender, UI* m_uiDraw);
	void compositeSSR(Mesh* m_meshRender, Camera* m_camera, GLuint TexCubemap);
	void recreateColorBuffer();

	std::vector<glm::vec3> createSampleKernel(std::uniform_real_distribution<GLfloat> randomFloats, std::default_random_engine generator);
	GLuint createNoiseTexture(std::uniform_real_distribution<GLfloat> randomFloats, std::default_random_engine generator);

	GLuint createSsaoColorBuffer();
	GLuint createSsaoColorBufferBlur();
	GLuint createSsrSceneColorBuffer();
	GLuint createSsrSceneColorBufferBlur();

	GLuint createSsaoFBO();
	GLuint createSsaoBlurFBO();
	GLuint createSsrFBO();
	GLuint createSsrBlurFBO();

	GLuint getColorBuffer() { return ssaoColorBuffer; }
	GLuint getBlurColorBuffer() { return ssaoColorBufferBlur; }

	GLuint getSsrColorBuffer() { return ssrColorBuffer; }
	GLuint getSsrBlurColorBuffer() { return ssrColorBufferBlur; }


	Shader* getSsaoShader() { return m_SSAO; }
	Shader* getSsrShader() { return m_SSR; }

private:
	int width = 640, height = 480;

	Shader* m_SSAO = 0;
	Shader* m_blurSSAO = 0;
	Shader* m_SSR = 0;
	Shader* m_blurSSR = 0;
	GBuffer* m_GBuffer;

	GLuint ssaoFBO = 0, ssaoBlurFBO = 0, ssrFBO = 0, ssrBlurFBO = 0;
	GLuint ssaoColorBuffer = 0, ssaoColorBufferBlur = 0, noiseTexture = 0, ssrColorBuffer = 0, ssrColorBufferBlur = 0;

	std::vector<glm::vec3> ssaoKernel;
};