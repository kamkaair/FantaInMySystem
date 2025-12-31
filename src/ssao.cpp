#include "ssao.h"

SSAO::SSAO(GBuffer* gbuffer, int inWidth, int inHeight)
	: m_GBuffer(gbuffer), width(inWidth), height(inHeight), Object(__FUNCTION__) {
	setupSSAO(); //constructSSAO(); constructed, when switching to the deferred rendering
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
	if (m_blurSSR != 0) { utils::deleteObject(m_blurSSR); }
}

void SSAO::constructSSAO() {
	// Load SSAO shaders
	if (m_SSAO == 0)
		m_SSAO = utils::makeShader("SSAO-Vert.glsl", "SSAO-Frag.glsl");

	if (m_blurSSAO == 0)
		m_blurSSAO = utils::makeShader("SSAO-Vert.glsl", "blurSSAO-Frag.glsl");

	if (m_SSR == 0)
		m_SSR = utils::makeShader("SSAO-Vert.glsl", "SSR-Frag.glsl");

	if (m_blurSSR == 0)
		m_blurSSR = utils::makeShader("SSAO-Vert.glsl", "blurSSR-Frag.glsl");
}

void SSAO::setupSSAO() {
	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
	std::default_random_engine generator;

	// SSAO texture framebuffer
	ssaoFBO = createSsaoFBO();
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	ssaoColorBuffer = createSsaoColorBuffer();

	// SSR Color Buffer
	ssrFBO = createSsrFBO();
	glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO);
	ssrColorBuffer = createSsrSceneColorBuffer();

	// SSR Blur framebuffer
	//ssrBlurFBO = createSsrBlurFBO();
	//glBindFramebuffer(GL_FRAMEBUFFER, ssrBlurFBO);
	//ssrColorBufferBlur = createSsrSceneColorBufferBlur();

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
	//glClear(GL_COLOR_BUFFER_BIT);
	m_blurSSAO->bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);

	m_meshRender->renderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAO::renderSSR(Camera* m_camera, Mesh* m_meshRender, UI* m_uiDraw) {
	// -------------------------------
	// SSR
	// -------------------------------

	glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	m_SSR->bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_GBuffer->getGNormal());
	m_SSR->setUniform("gNormal", 0);

	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, m_GBuffer->getGPosition());
	//m_SSR->setUniform("gPosition", 1);

	glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, m_GBuffer->getLightingTex());
	glBindTexture(GL_TEXTURE_2D, m_GBuffer->getIndirectSpecular());
	m_SSR->setUniform("colorBuffer", 1);

	//glActiveTexture(GL_TEXTURE3);
	//glBindTexture(GL_TEXTURE_2D, m_GBuffer->getGMetallicRoughness());
	//m_SSR->setUniform("gMetalRough", 3);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_GBuffer->getGDepth());
	m_SSR->setUniform("depthMap", 2);

	m_SSR->setUniform("SCR_WIDTH", float(width));
	m_SSR->setUniform("SCR_HEIGHT", float(height));
	//std::cout << m_camera->getNear() << " " << m_camera->getFar() << std::endl;

	//m_SSR->setUniform("view", glm::vec3(0.0f, 0.0f, 0.0f));
	m_SSR->setUniform("invProjection", glm::inverse(m_camera->getProjectionMatrix()));
	m_SSR->setUniform("projection", m_camera->getProjectionMatrix());
	//m_SSR->setUniform("far", m_camera->getFar());
	//m_SSR->setUniform("near", m_camera->getNear());

	//m_SSR->setUniform("MAX_STEPS", m_uiDraw->getSSR_MaxSteps());
	//m_SSR->setUniform("stepSize", m_uiDraw->getSSR_StepSize());
	m_SSR->setUniform("thickness", m_uiDraw->getSSR_Thickness());
	//m_SSR->setUniform("bias", m_uiDraw->getSSR_bias());

	//m_SSR->setUniform("cameraPos", m_camera->getPosition());
	//m_SSR->setUniform("screenSize", glm::vec2(width, height));


	m_meshRender->renderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// -------------------------------
	// BLUR
	// -------------------------------

	//Apply the blur to the SSAO texture - SSAO blur technique
	//glBindFramebuffer(GL_FRAMEBUFFER, ssrBlurFBO);
	////glClear(GL_COLOR_BUFFER_BIT);
	//m_blurSSAO->bind();
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, ssrColorBuffer);
	//m_blurSSAO->setUniform("ssaoInput", 0);

	//m_meshRender->renderQuad();
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	

	//// Horizontal and vertical blur for SSR!
	//
	//// Apply blur horizontally to the SSR
	//glBindFramebuffer(GL_FRAMEBUFFER, ssrBlurFBO);
	////glClear(GL_COLOR_BUFFER_BIT);
	//m_blurSSR->bind();

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, ssrColorBuffer);

	//m_blurSSR->setUniform("ssrInput", 0);
	//m_blurSSR->setUniform("direction", glm::vec2(1.0f, 0.0f));
	////m_blurSSR->setUniform("resolution", (float)width);

	//m_meshRender->renderQuad();

	//// Apply the blur vertically
	//glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO);
	//m_blurSSR->bind();
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, ssrColorBufferBlur);
	//m_blurSSR->setUniform("ssrInput", 0);
	//m_blurSSR->setUniform("direction", glm::vec2(0.0f, 1.0f));
	////m_blurSSR->setUniform("resolution", (float)height);

	//m_meshRender->renderQuad();
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAO::compositeSSR(Mesh* m_meshRender, Texture* TexHDRI, GLuint TexCubemap)
{
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_ONE, GL_ONE); // additive blend
	//glEnable(GL_BLEND);
	//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);

	m_GBuffer->getCompositeShader()->bind();

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, lightingTex);
	//m_GBuffer->getCompositeShader()->setUniform("uLightingTex", 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssrColorBuffer);
	m_GBuffer->getCompositeShader()->setUniform("uSSR", 0);

	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, m_GBuffer->getGMetallicRoughness());
	//m_GBuffer->getCompositeShader()->setUniform("uRoughMetal", 1);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_GBuffer->getDiffuse());
	m_GBuffer->getCompositeShader()->setUniform("uDirectDiffuse", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_GBuffer->getSpecular());
	m_GBuffer->getCompositeShader()->setUniform("uDirectSpec", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_GBuffer->getIndirectDiffuse());
	m_GBuffer->getCompositeShader()->setUniform("uIndirectDiffuse", 3);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, m_GBuffer->getIndirectSpecular());
	m_GBuffer->getCompositeShader()->setUniform("uIndirectSpecFallback", 4);

	//m_GBuffer->getCompositeShader()->setUniform("useRoughnessMask", true);

	m_meshRender->renderQuad();

	glEnable(GL_DEPTH_TEST);

	//glDisable(GL_BLEND);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

GLuint SSAO::createSsrSceneColorBuffer() {
	glGenTextures(1, &ssrColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssrColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssrColorBuffer, 0);
	GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSR Framebuffer not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return ssrColorBuffer;
}

GLuint SSAO::createSsrSceneColorBufferBlur() {
	glGenTextures(1, &ssrColorBufferBlur);
	glBindTexture(GL_TEXTURE_2D, ssrColorBufferBlur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssrColorBufferBlur, 0);
	GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSR Framebuffer not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return ssrColorBufferBlur;
}

GLuint SSAO::createSsrFBO() {
	glGenFramebuffers(1, &ssrFBO); return ssrFBO;
}

GLuint SSAO::createSsaoFBO() {
	glGenFramebuffers(1, &ssaoFBO); return ssaoFBO;
}

GLuint SSAO::createSsaoBlurFBO() {
	glGenFramebuffers(1, &ssaoBlurFBO); return ssaoBlurFBO;
}

GLuint SSAO::createSsrBlurFBO() {
	glGenFramebuffers(1, &ssrBlurFBO); return ssrBlurFBO;
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

	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);

	if (ssaoColorBufferBlur != 0) { glDeleteTextures(1, &ssaoColorBufferBlur); ssaoColorBufferBlur = 0; }
	ssaoColorBufferBlur = createSsaoColorBufferBlur();

	glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO);

	if (ssrColorBuffer != 0) { glDeleteTextures(1, &ssrColorBuffer); ssrColorBuffer = 0; }
	ssrColorBuffer = createSsrSceneColorBuffer();

	m_SSAO->setUniform("noiseScale", glm::vec2(width / 4.0f, height / 4.0f)); // new noiseScale resolution

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}