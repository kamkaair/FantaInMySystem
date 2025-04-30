#include "ssao.h"

SSAO::SSAO(GBuffer* gbuffer, int inWidth, int inHeight)
	: m_GBuffer(gbuffer), width(inWidth), height(inHeight), Object(__FUNCTION__) {
	setupSSAO(); constructSSAO();
}

SSAO::~SSAO() {
	if (!m_SSAO == 0) {
		delete m_SSAO;
		m_SSAO = 0;
	}
}

void SSAO::constructSSAO() {
	// Load SSAO shaders
	if (m_SSAO == 0) {
		std::string SSAOVertSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/SSAO-Vert.glsl");
		std::string SSAOFragSource = utils::loadShader(std::string(ASSET_DIR) + "/shaders/SSAO-Frag.glsl");
		m_SSAO = new Shader(SSAOVertSource, SSAOFragSource);
	}
}

void SSAO::deconstructSSAO() {
	//m_SSAO->deleteShader();
	if (!m_SSAO == 0) {
		delete m_SSAO;
		m_SSAO = 0;
	}
}

void SSAO::setupSSAO() {
	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
	std::default_random_engine generator;

	ssaoFBO = createSsaoFBO();
	ssaoKernel = createSampleKernel(randomFloats, generator);

	noiseTexture = createNoiseTexture(randomFloats, generator);
	ssaoColorBuffer = createSsaoColorBuffer();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAO::renderSSAO(Camera* m_camera, UI* m_uiDraw, Mesh* m_meshRender) {
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
	for (unsigned int i = 0; i < 64; ++i)
		m_SSAO->setUniform("samples[" + std::to_string(i) + "]", ssaoKernel[i]);

	m_SSAO->setUniform("projection", m_camera->getProjectionMatrix());

	m_SSAO->setUniform("kernelSize", m_uiDraw->getKernelSize());
	m_SSAO->setUniform("radius", m_uiDraw->getRadius());
	m_SSAO->setUniform("bias", m_uiDraw->getBias());

	m_SSAO->setUniform("noiseScale", glm::vec2(width / 4.0f, height / 4.0f));

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
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return noiseTexture;
}

GLuint SSAO::createSsaoFBO() {
	glGenFramebuffers(1, &ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

	return ssaoFBO;
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