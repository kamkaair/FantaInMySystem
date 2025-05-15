#include "ssao.h"

SSAO::SSAO(GBuffer* gbuffer, int inWidth, int inHeight)
	: m_GBuffer(gbuffer), width(inWidth), height(inHeight), Object(__FUNCTION__) {
	setupSSAO(); //constructSSAO();
}

SSAO::~SSAO() {
	deconstructSSAO();
	if (ssaoBlurFBO != 0) { glDeleteFramebuffers(1, &ssaoBlurFBO); ssaoBlurFBO = 0; }
	if (ssaoColorBufferBlur != 0) { glDeleteTextures(1, &ssaoColorBufferBlur); ssaoColorBufferBlur = 0; }

	if (ssaoFBO != 0) { glDeleteFramebuffers(1, &ssaoFBO); ssaoFBO = 0; }
	if (noiseTexture != 0) { glDeleteTextures(1, &noiseTexture); noiseTexture = 0; }
	if (ssaoColorBuffer != 0) { glDeleteTextures(1, &ssaoColorBuffer); ssaoColorBuffer = 0; }
	for (int i = 0; i < ssaoKernel.size(); i++) {
		ssaoKernel[i] = glm::vec3{ 0.0f };
	}
}

void SSAO::deconstructSSAO() {
	if (m_SSAO != 0) { utils::deleteObject(m_SSAO); }
	if (m_blurSSAO != 0) { utils::deleteObject(m_blurSSAO); }
	if (m_SSR != 0) { utils::deleteObject(m_SSR); }
}

void SSAO::constructSSAO() {
	// Load SSAO shaders
	if (m_SSAO == 0)
		m_SSAO = utils::makeShader("SSAO-Vert.glsl", "SSAO-Frag.glsl");

	if (m_blurSSAO == 0)
		m_blurSSAO = utils::makeShader("SSAO-Vert.glsl", "blurSSAO-Frag.glsl");

	if (m_SSR == 0)
		m_SSR = utils::makeShader("SSAO-Vert.glsl", "SSR-Frag.glsl");
}

void SSAO::setupSSAO() {
	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
	std::default_random_engine generator;

	// SSAO texture framebuffer
	ssaoFBO = createSsaoFBO();
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	ssaoColorBuffer = createSsaoColorBuffer();

	// SSR
	ssrFBO = createSsrFBO();
	glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO);
	ssrColorBuffer = createSsrColorBuffer();

	// Blur framebuffer
	ssaoBlurFBO = createSsaoBlurFBO();
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	ssaoColorBufferBlur = createSsaoColorBufferBlur();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	ssaoKernel = createSampleKernel(randomFloats, generator);
	noiseTexture = createNoiseTexture(randomFloats, generator);
}

void SSAO::renderSSAO(Camera* m_camera, UI* m_uiDraw, Mesh* m_meshRender, int inWidth, int inHeight, int samples) {
	width = inWidth;
	height = inHeight;

	// SSAO texture
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	m_SSAO->bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	m_SSAO->setUniform("texNoise", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_GBuffer->getGPosition());
	m_SSAO->setUniform("gPosition", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_GBuffer->getGNormal());
	m_SSAO->setUniform("gNormal", 2);

	// Send kernel + rotation 
	for (unsigned int i = 0; i < samples; ++i)
		m_SSAO->setUniform("samples[" + std::to_string(i) + "]", ssaoKernel[i]);

	m_SSAO->setUniform("projection", m_camera->getProjectionMatrix());

	m_meshRender->renderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Apply the blur to the SSAO texture
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	m_blurSSAO->bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	m_meshRender->renderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// SSR
	glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO);
	m_SSR->bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_GBuffer->getGPosition());
	m_SSR->setUniform("gPosition", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_GBuffer->getGNormal());
	m_SSR->setUniform("gNormal", 1);

	m_SSR->setUniform("projection", m_camera->getProjectionMatrix());
	m_meshRender->renderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

float SSAOLerp(float a, float b, float f)
{
	return a + f * (b - a);
}

std::vector<glm::vec3> SSAO::createSampleKernel(std::uniform_real_distribution<GLfloat> randomFloats, std::default_random_engine generator) {
	for (unsigned int i = 0; i < 64; ++i)
	{
		glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		float scale = float(i) / 64.0f;

		// scale samples s.t. they're more aligned to center of kernel
		scale = SSAOLerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
		//std::cout << glm::to_string(sample) << " also: " << i << std::endl;
	}
	return ssaoKernel;
}

GLuint SSAO::createNoiseTexture(std::uniform_real_distribution<GLfloat> randomFloats, std::default_random_engine generator) {
	std::vector<glm::vec3> ssaoNoise;

	for (unsigned int i = 0; i < 16; i++)
	{
		glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
		ssaoNoise.push_back(noise);
	}
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return noiseTexture;
}

GLuint SSAO::createSsrColorBuffer() {
	glGenTextures(1, &ssrColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssrColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssrColorBuffer, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSR Framebuffer not complete!" << std::endl;

	return ssrColorBuffer;
}

GLuint SSAO::createSsrFBO() {
	glGenFramebuffers(1, &ssrFBO);

	return ssrFBO;
}

GLuint SSAO::createSsaoFBO() {
	glGenFramebuffers(1, &ssaoFBO);

	return ssaoFBO;
}

GLuint SSAO::createSsaoBlurFBO() {
	glGenFramebuffers(1, &ssaoBlurFBO);

	return ssaoBlurFBO;
}

GLuint SSAO::createSsaoColorBuffer() {
	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Framebuffer not complete!" << std::endl;

	return ssaoColorBuffer;
}

GLuint SSAO::createSsaoColorBufferBlur() {
	glGenTextures(1, &ssaoColorBufferBlur);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Framebuffer not complete!" << std::endl;

	return ssaoColorBufferBlur;
}

void SSAO::recreateColorBuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

	if (ssaoColorBuffer != 0) { glDeleteTextures(1, &ssaoColorBuffer); ssaoColorBuffer = 0; }
	ssaoColorBuffer = createSsaoColorBuffer();
	if (ssaoColorBufferBlur != 0) { glDeleteTextures(1, &ssaoColorBufferBlur); ssaoColorBufferBlur = 0; }
	ssaoColorBufferBlur = createSsaoColorBufferBlur();
	m_SSAO->setUniform("noiseScale", glm::vec2(width / 4.0f, height / 4.0f)); // new noiseScale resolution

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}