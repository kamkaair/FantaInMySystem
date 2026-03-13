#include "ssao.h"

SSAO::SSAO(GBuffer* gbuffer, int inWidth, int inHeight)
	: m_GBuffer(gbuffer), width(inWidth), height(inHeight), Object(__FUNCTION__) {
	setupSSAO(); //constructSSAO(); constructed, when switching to the deferred rendering
}

SSAO::~SSAO() {
	deconstructSSAO();
	deconstructSSR();
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
}

void SSAO::deconstructSSR() {
	if (m_SSR != 0) { utils::deleteObject(m_SSR); }
	//if (m_blurSSR != 0) { utils::deleteObject(m_blurSSR); }
	if (m_SSR_TA != 0) { utils::deleteObject(m_SSR_TA); }
}

void SSAO::constructSSAO() {
	// Load SSAO shaders
	if (m_SSAO == 0)
		m_SSAO = utils::makeShader("SSAO-Vert.glsl", "SSAO-Frag.glsl");

	if (m_blurSSAO == 0)
		m_blurSSAO = utils::makeShader("SSAO-Vert.glsl", "blurSSAO-Frag.glsl");
}

void SSAO::constructSSR() {
	// Load SSAO shaders
	if (m_SSR == 0)
		m_SSR = utils::makeShader("SSAO-Vert.glsl", "SSR-Frag.glsl");

	//if (m_blurSSR == 0)
		//m_blurSSR = utils::makeShader("SSAO-Vert.glsl", "blurSSR-Frag.glsl");

	if (m_SSR_TA == 0)
		m_SSR_TA = utils::makeShader("SSAO-Vert.glsl", "SSR-TAF.glsl");
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

	// SSR Temporal Accumulation FBO
	createSSR_HistoryFramebuffer();
	createTemporalBuffers();

	//// SSR Blur framebuffer
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

	updateSSAOUniforms();

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

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_GBuffer->getIndirectSpecular());
	m_SSR->setUniform("colorBuffer", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_GBuffer->getGDepth());
	m_SSR->setUniform("depthMap", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_GBuffer->getGMetallicRoughness());
	m_SSR->setUniform("gMetallicRoughness", 3);

	m_SSR->setUniform("SCR_WIDTH", float(width));
	m_SSR->setUniform("SCR_HEIGHT", float(height));

	m_SSR->setUniform("invProjection", glm::inverse(m_camera->getProjectionMatrix()));
	m_SSR->setUniform("projection", m_camera->getProjectionMatrix());

	m_SSR->setUniform("frameIndex", frameIndex);

	m_meshRender->renderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Temporal Accumulation
	if(m_ssrSettings.useTA) {
		glBindFramebuffer(GL_FRAMEBUFFER, getSSRHistoryWriteFBO());
		m_SSR_TA->bind();

		renderSSR_TA(m_camera, m_meshRender);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// -------------------------------
	// BLUR
	// -------------------------------

	////Apply the blur to the SSAO texture - SSAO blur technique
	//glBindFramebuffer(GL_FRAMEBUFFER, ssrBlurFBO);
	////glClear(GL_COLOR_BUFFER_BIT);
	//m_blurSSR->bind();
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, ssrColorBuffer);
	//m_blurSSR->setUniform("colorTexture", 0);

	//m_meshRender->renderQuad();
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAO::renderSSR_TA(Camera* m_camera, Mesh* m_meshRender) {
	// -------------------------------
	// SSR Temporal Accumulation
	// -------------------------------

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssrColorBuffer);
	m_SSR_TA->setUniform("uSSRCurrent", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, getSSRHistoryRead());
	m_SSR_TA->setUniform("uSSRHistory", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_GBuffer->getGDepth());
	m_SSR_TA->setUniform("depthMap", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_GBuffer->getGNormal());
	m_SSR_TA->setUniform("gNormal", 3);

	// Previous depth
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, prevDepthTex);
	m_SSR_TA->setUniform("uPrevDepth", 4);

	// Previous normal
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, prevNormalTex);
	m_SSR_TA->setUniform("uPrevNormal", 5);

	m_SSR_TA->setUniform("near", m_camera->getNear());
	m_SSR_TA->setUniform("far", m_camera->getFar());

	glm::mat4 currView = m_camera->getViewMatrix();
	glm::mat4 currProj = m_camera->getProjectionMatrix();

	if (firstFrame)
	{
		prevView = currView;
		prevProj = currProj;
		firstFrame = false;
	}

	m_SSR_TA->setUniform("invProjection", glm::inverse(m_camera->getProjectionMatrix()));
	m_SSR_TA->setUniform("prevProjection", prevProj);

	m_meshRender->renderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// -------------------------------
	// SSR History - Copying
	// -------------------------------

	swapSSRHistory();

	// Copy depth to prevDepth
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_GBuffer->getGBuffer());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDepthFBO);

	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	// Copy normal
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_GBuffer->getGBuffer());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevNormalFBO);

	glReadBuffer(GL_COLOR_ATTACHMENT1); // gNormal
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	prevView = currView;
	prevProj = currProj;
	frameIndex++; // frame index used for the random
}

void SSAO::renderCompositeShader(Mesh* m_meshRender, Camera* m_camera, HDRI* m_HDRI, UI* m_uiDraw)
{
	// -------------------------------
	// SSR Composite - Final Image
	// -------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);

	m_GBuffer->getCompositeShader()->bind();

	GLuint ssrSelect = m_ssrSettings.useTA ? getSSRHistoryRead() : ssrColorBuffer;

	glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, ssrColorBuffer); //getSSRHistoryRead
	glBindTexture(GL_TEXTURE_2D, ssrSelect);
	m_GBuffer->getCompositeShader()->setUniform("uSSR", 0);

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

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, m_GBuffer->getGDepth());
	m_GBuffer->getCompositeShader()->setUniform("uDepth", 5);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_HDRI->getCubemapTexture());
	m_GBuffer->getCompositeShader()->setUniform("uSkybox", 6);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, m_HDRI->getBackgroundTexture()->getTextureId());
	m_GBuffer->getCompositeShader()->setUniform("uBackgroundTex", 7);

	m_GBuffer->getCompositeShader()->setUniform("backgroundMode", m_uiDraw->getBackgroundMode());
	m_GBuffer->getCompositeShader()->setUniform("invProjection", glm::inverse(m_camera->getProjectionMatrix()));
	m_GBuffer->getCompositeShader()->setUniform("invView", glm::inverse(m_camera->getViewMatrix()));

	m_meshRender->renderQuad();

	glEnable(GL_DEPTH_TEST);
}

void SSAO::updateSSAOUniforms() {
	if (!m_ssaoSettings.dirty)
		return;

	m_SSAO->bind();
	m_SSAO->setUniform("kernelSize", m_ssaoSettings.kernelSize);
	m_SSAO->setUniform("radius", m_ssaoSettings.radius);
	m_SSAO->setUniform("bias", m_ssaoSettings.bias);

	m_GBuffer->getLightPass()->bind();
	m_GBuffer->getLightPass()->setUniform("aoTone", m_ssaoSettings.clampedMidTones);
	m_GBuffer->getLightPass()->setUniform("useSSAO", m_ssaoSettings.useSSAO);
	m_GBuffer->getLightPass()->setUniform("aoStrength", m_ssaoSettings.occlusionStrength);

	m_ssaoSettings.dirty = false;
}

void SSAO::updateSSRUniforms() {
	if (!m_ssrSettings.useTA) {
		resetTA_SSR();
		return;
	}

	if (!m_ssrSettings.dirty)
		return;

	m_SSR->bind();
	m_SSR->setUniform("useRoughnessScatterSSR", m_ssrSettings.useRayScattering);
	m_SSR->setUniform("maxSteps", m_ssrSettings.maxSteps);
	m_SSR->setUniform("thickness", m_ssrSettings.thickness);
	m_SSR->setUniform("rayDirMin", m_ssrSettings.rayDirMin);
	m_SSR->setUniform("useBinaryRefinement", m_ssrSettings.useBinaryRefinement);

	m_ssrSettings.dirty = false;
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

void SSAO::createSSR_HistoryFramebuffer() {
	glGenFramebuffers(2, ssrHistoryFBO);
	glGenTextures(2, ssrTemporalBuffer);

	for (int i = 0; i < 2; ++i)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, ssrHistoryFBO[i]);

		glBindTexture(GL_TEXTURE_2D, ssrTemporalBuffer[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssrTemporalBuffer[i], 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "SSR history FBO incomplete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAO::createTemporalBuffers()
{
	// prevDepth
	glGenTextures(1, &prevDepthTex);
	glBindTexture(GL_TEXTURE_2D, prevDepthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0,GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenFramebuffers(1, &prevDepthFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, prevDepthFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, prevDepthTex, 0);

	// prevNormal
	glGenTextures(1, &prevNormalTex);
	glBindTexture(GL_TEXTURE_2D, prevNormalTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenFramebuffers(1, &prevNormalFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, prevNormalFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, prevNormalTex, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

	if ((ssrTemporalBuffer[0] != 0) && (ssrTemporalBuffer[1] != 0)) {
		for (int i = 0; i < 2; i++) {
			glBindFramebuffer(GL_FRAMEBUFFER, ssrHistoryFBO[i]);
			glDeleteTextures(1, &ssrTemporalBuffer[i]);
			ssrTemporalBuffer[i] = 0;
		}
	}
	createSSR_HistoryFramebuffer();
	createTemporalBuffers();

	m_SSAO->setUniform("noiseScale", glm::vec2(width / 4.0f, height / 4.0f)); // new noiseScale resolution

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAO::resetTA_SSR() {
	ssrHistoryIndex = 0;
	frameIndex = 0;
	firstFrame = true;

	for (int i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, ssrHistoryFBO[i]);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	// Clear depth
	glBindFramebuffer(GL_FRAMEBUFFER, prevDepthFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	// Clear normal
	glBindFramebuffer(GL_FRAMEBUFFER, prevNormalFBO);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/*if ((ssrTemporalBuffer[0] != 0) && (ssrTemporalBuffer[1] != 0)) {
		for (int i = 0; i < 2; i++) {
			glBindFramebuffer(GL_FRAMEBUFFER, ssrHistoryFBO[i]);
			glDeleteTextures(1, &ssrTemporalBuffer[i]);
			ssrTemporalBuffer[i] = 0;
		}
	}*/

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}