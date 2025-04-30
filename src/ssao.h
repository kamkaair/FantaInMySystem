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
	void deconstructSSAO();
	void setupSSAO();
	void renderSSAO(Camera* m_camera, UI* m_uiDraw, Mesh* m_meshRender);

	std::vector<glm::vec3> createSampleKernel(std::uniform_real_distribution<GLfloat> randomFloats, std::default_random_engine generator);
	GLuint createNoiseTexture(std::uniform_real_distribution<GLfloat> randomFloats, std::default_random_engine generator);
	GLuint createSsaoFBO();
	GLuint createSsaoColorBuffer();

	GLuint getColorBuffer() { return ssaoColorBuffer; }

private:
	Shader* m_SSAO;
	GBuffer* m_GBuffer;

	GLuint ssaoFBO = 0, ssaoColorBuffer = 0, noiseTexture = 0;

	std::vector<glm::vec3> ssaoKernel;
	int width = 640, height = 480;
};